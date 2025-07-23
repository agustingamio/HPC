#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "../mpi_utils.h"
#include "../mpi_types.h"
#include "../ticket.h"
#include "../frequency.h"
#include "../stop_graph.h"

int get_slave_for_variantId(array_node* slaves_array, const int variant_id, const int slaves_count) {
    const int slave = find_slave(slaves_array, variant_id, slaves_count);

    if (slave == -1) {
        const int less_overloaded_slave = find_less_overloaded_slave(slaves_array, slaves_count);
        add_variant_to_array(slaves_array, variant_id, less_overloaded_slave);
        return less_overloaded_slave;
    }

    return slave;
}

void initialize_parameters(Frequency** frequencies, int* freq_count,
                           StopGraph** graph, int* stops_count,
                           Ticket** tickets, int* tickets_count,
                           array_node** slaves_array, int* comm_size)
{
    MPI_Comm_size(MPI_COMM_WORLD, comm_size);

    // Load frequencies
    if (calculate_frequency_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv", frequencies, freq_count) != 0) {
        printf("Failed to read CSV\n");
    }

    // Load graph
    if (load_stop_graph_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante_guardado.csv", graph, stops_count) != 0) {
        // Fall back to expensive calculation
        if (create_stop_graph_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv", graph, stops_count) != 0) {
            printf("Error loading stop graph\n");
        }

        // Save result for future runs
        save_stop_graph_to_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante_guardado.csv", *graph, *stops_count);
    }

    // Load tickets
    if (read_tickets_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\tickets_output.csv", tickets, tickets_count) != 0) {
        printf("Failed to tickets CSV\n");
    }

    *slaves_array = create_slaves_array(*comm_size);
}

void send_structures(Frequency* frequencies, const int freq_count,
                     StopGraph* graph, const int stops_count,
                     const MPITypes types)
{
    MPI_Bcast((void*)&freq_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(frequencies, freq_count, types.frequency_type, 0, MPI_COMM_WORLD);
    MPI_Bcast((void*)&stops_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(graph, stops_count, types.stop_graph_type, 0, MPI_COMM_WORLD);
}

void process_tickets(const Ticket* tickets, const int tickets_count, array_node* slaves_array, const int comm_size, const MPITypes types) {
    for (int i = 0; i < tickets_count; i++) {
        const int slave = get_slave_for_variantId(slaves_array, tickets[i].variant_id, comm_size);
        MPI_Send(&tickets[i], 1, types.ticket_type, slave + 1, 0, MPI_COMM_WORLD);

        _sleep(1);
    }
}

void send_stop_signal(const MPITypes types, const int comm_size) {
    const Ticket ticket;
    const enum tag_types SHUTDOWN = shutdown_signal;
    for (int i = 1; i < comm_size; i++) {
        MPI_Send(&ticket, 1, types.ticket_type, i, SHUTDOWN, MPI_COMM_WORLD);
    }
}

void main_master(const MPITypes types) {
    int tickets_count = 0, comm_size = 0, freq_count = 0, stops_count = 0;
    Ticket* tickets = NULL;
    Frequency* frequencies = NULL;
    StopGraph* graph = NULL;
    array_node* slaves_array = NULL;

    initialize_parameters(&frequencies, &freq_count, &graph, &stops_count, &tickets, &tickets_count, &slaves_array, &comm_size);
    send_structures(frequencies, freq_count, graph, stops_count, types);
    process_tickets(tickets, tickets_count, slaves_array, comm_size, types);
    send_stop_signal(types, comm_size);

    free_tickets(tickets);
    free_stop_graph(graph);
    free_frequencies(frequencies);
    free_slaves_array(slaves_array, comm_size);
}
