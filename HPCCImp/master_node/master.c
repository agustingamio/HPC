#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "../mpi_utils.h"
#include "../mpi_types.h"
#include "../ticket.h"
#include "../frequency.h"
#include "../stop_graph.h"

int get_slave_for_variantId(array_node* slaves_array, const int variant_id, const int slaves_count, int* update){
    const int slave = find_slave(slaves_array, variant_id, slaves_count, update);

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
                           array_node** slaves_array, array_node** statistical_array,
                           int* comm_size, const int statistical_nodes_amount)
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

    *slaves_array = create_slaves_array(*comm_size - 2);
    // TODO: Change name of create_slaves_array, and related functions, to a generic one, since its now used for statistical nodes as well
    *statistical_array = create_slaves_array(statistical_nodes_amount);
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

void send_variant_data(const Ticket ticket, const int from_rank, const int rank_to, const MPITypes types) {
    // If performance is decreased due to 4 sends, we could send the sender and receiver inside the ticket, however that approach would be less clean
    MPI_Send(&ticket, 1, types.ticket_type, from_rank, send_updates, MPI_COMM_WORLD);
    MPI_Send(&rank_to, 1, MPI_INT, from_rank, send_updates, MPI_COMM_WORLD);
    MPI_Send(&ticket, 1, types.ticket_type, rank_to, recv_updates, MPI_COMM_WORLD);
    MPI_Send(&from_rank, 1, MPI_INT, rank_to, recv_updates, MPI_COMM_WORLD);
}

void process_tickets(const Ticket* tickets, const int tickets_count, array_node* slaves_array, array_node* statistical_array, const int comm_size, const MPITypes types, const int statistical_nodes_amount) {
    for (int i = 0; i < tickets_count; i++) {
        int update = 0;
        const int slave_rank = get_slave_for_variantId(slaves_array, tickets[i].variant_id, comm_size - 2, &update) + 1;
        const int statistical_rank = get_slave_for_variantId(statistical_array, tickets[i].variant_id, comm_size - 2, &update) + (comm_size - statistical_nodes_amount);

        if (update == 2) {
            send_variant_data(tickets[i], slave_rank, statistical_rank, types);
            printf("Graph updated for variant %d\n", tickets[i].variant_id);
        }

        MPI_Send(&tickets[i], 1, types.ticket_type, slave_rank, ticket_to_process, MPI_COMM_WORLD);
        MPI_Send(&tickets[i], 1, types.ticket_type, statistical_rank, ticket_to_process, MPI_COMM_WORLD);
        printf("Ticket processed for variant %d\n", tickets[i].variant_id);
        fflush(stdout);

        _sleep(40);
    }
}

void send_stop_signal(const MPITypes types, const int comm_size) {
    const Ticket ticket;
    const enum tag_types SHUTDOWN = shutdown_signal;
    for (int i = 1; i < comm_size; i++) {
        MPI_Send(&ticket, 1, types.ticket_type, i, SHUTDOWN, MPI_COMM_WORLD);
    }
}

void main_master(const MPITypes types, const int statistical_nodes_amount) {
    int tickets_count = 0, comm_size = 0, freq_count = 0, stops_count = 0;
    Ticket* tickets = NULL;
    Frequency* frequencies = NULL;
    StopGraph* graph = NULL;
    array_node* slaves_array = NULL, *statistical_array = NULL;

    initialize_parameters(&frequencies, &freq_count, &graph, &stops_count, &tickets, &tickets_count, &slaves_array,  &statistical_array, &comm_size, statistical_nodes_amount);
    send_structures(frequencies, freq_count, graph, stops_count, types);
    process_tickets(tickets, tickets_count, slaves_array, statistical_array, comm_size, types, statistical_nodes_amount);
    send_stop_signal(types, comm_size);

    free_tickets(tickets);
    free_stop_graph(graph);
    free_frequencies(frequencies);
    free_slaves_array(slaves_array, comm_size);
}
