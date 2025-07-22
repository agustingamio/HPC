#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "frequency.h"
#include "mpi_types.h"
#include "stop_graph.h"
#include "ticket.h"
#include "master_node/master.h"

int master() {
    // INIT MPI and send the structures
    int rank, comm_size;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    MPITypes types;
    create_tm_type(&types.tm_type);
    create_ticket_type(&types.ticket_type, types.tm_type);
    create_stop_graph_type(&types.stop_graph_type, types.tm_type);
    create_frequency_type(&types.frequency_type, types.tm_type);

    int freq_count = 0;
    int stops_count = 0;

    Frequency* frequencies = NULL;
    StopGraph* graph = NULL;

    if (rank == 0) {
        // Load frequencies
        if (calculate_frequency_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv", &frequencies, &freq_count) != 0) {
            printf("Failed to read CSV\n");
            return 1;
        }

        // Load graph
        if (load_stop_graph_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante_guardado.csv", &graph, &stops_count) != 0) {
            // Fall back to expensive calculation
            if (create_stop_graph_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv", &graph, &stops_count) != 0) {
                printf("Error loading stop graph\n");
                return 1;
            }

            // Save result for future runs
            save_stop_graph_to_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante_guardado.csv", graph, stops_count);
        }
    }

    MPI_Bcast(frequencies, freq_count, types.frequency_type, 0, MPI_COMM_WORLD);
    MPI_Bcast(graph, stops_count, types.stop_graph_type, 0, MPI_COMM_WORLD);


    if (rank == 0) {
        // MASTER LOGIC
        main_master(types);
    } else if (rank < comm_size) {
        // SLAVE LOGIC
    } else {
        // STATISTIC LOGIC
    }

    // Clean up types
    MPI_Type_free(&types.ticket_type);
    MPI_Type_free(&types.stop_graph_type);
    MPI_Type_free(&types.frequency_type);
    MPI_Type_free(&types.tm_type);

    MPI_Finalize();
    return 0;
}
