#include "checkpoints.h"

#include <stdio.h>
#include <stdlib.h>

#include "../mpi_types.h"
#include "../mpi_utils.h"
#include "../ticket_processor.h"

void create_checkpoint_proc(const int variant_count, StopGraph* graph, const int stops_count, const MPITypes types) {
    // Update the graph with the received data
    for (int i = 0; i < variant_count; i++) {
        recv_graph_updates(graph, stops_count, types);
    }

    // Delete the previous checkpoint if it exists
    if (remove("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante_guardado.csv") != 0) {
        printf("Error deleting previous checkpoint\n");
    }

    // Save the graph to a csv
    if (save_stop_graph_to_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante_guardado.csv", graph, stops_count) != 0) {
        printf("Error saving checkpoint\n");
    }
}

void checkpoint_process(StopGraph* graph, const int stops_count, MPITypes types) {
    while (1) {
        int variant_count = 0;
        MPI_Status status;

        MPI_Recv(&variant_count,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        const enum tag_types tag = status.MPI_TAG;

        if (tag == create_checkpoint) {
            create_checkpoint_proc(variant_count, graph, stops_count, types);
        } else if (tag == shutdown_signal) {
            break;
        }
    }
}


void main_checkpoint(const MPITypes types, const int statistical_nodes_amount) {
    int freq_count = 0, stops_count = 0;
    Frequency* frequencies = NULL;
    StopGraph* graph = NULL;

    receive_structures(&frequencies, &freq_count, &graph, &stops_count, types);
    checkpoint_process(graph, stops_count, types);

    free(frequencies);
    free(graph);
}
