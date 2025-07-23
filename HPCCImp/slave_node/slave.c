//
// Created by agamio on 7/22/2025.
//

#include "slave.h"

#include <stdlib.h>

#include "../mpi_types.h"
#include "../mpi_utils.h"
#include "../ticket.h"
#include "../ticket_processor.h"

void receive_structures(Frequency** frequencies, int* freq_count,
                        StopGraph** graph, int* stops_count,
                        const MPITypes types)
{
    MPI_Bcast(freq_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    *frequencies = (Frequency*)malloc(*freq_count * sizeof(Frequency));
    MPI_Bcast(*frequencies, *freq_count, types.frequency_type, 0, MPI_COMM_WORLD);

    MPI_Bcast(stops_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    *graph = (StopGraph*)malloc(*stops_count * sizeof(StopGraph));
    MPI_Bcast(*graph, *stops_count, types.stop_graph_type, 0, MPI_COMM_WORLD);
}

void send_graph_updates(StopGraph* graph, const int stops_count,
                        const MPITypes types)
{
    // WAITING FOR PREDICTOR IMPLEMENTATION
}

void message_process(const Frequency* frequencies, const int freq_count,
                     StopGraph* graph, const int stops_count,
                     const MPITypes types)
{
    while (1) {
        Ticket ticket;
        MPI_Status status;

        MPI_Recv(&ticket,1,types.ticket_type,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        const enum tag_types tag = status.MPI_TAG;

        if (tag == ticket_to_process) {
            process_ticket(ticket, graph, stops_count, frequencies, freq_count);
        } else if (tag == send_updates) {
            send_graph_updates(graph, stops_count, types);
        } else if (tag == shutdown_signal) {
            break;
        }
    }
}

void main_slave(const MPITypes types) {
    int freq_count = 0, stops_count = 0;
    Frequency* frequencies = NULL;
    StopGraph* graph = NULL;

    receive_structures(&frequencies, &freq_count, &graph, &stops_count, types);
    message_process(frequencies, freq_count, graph, stops_count, types);

    free(frequencies);
    free(graph);
}
