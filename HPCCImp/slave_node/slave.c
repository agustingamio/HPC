//
// Created by agamio on 7/22/2025.
//

#include "slave.h"

#include <stdlib.h>

#include "../mpi_types.h"
#include "../mpi_utils.h"
#include "../ticket.h"
#include "../ticket_processor.h"

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
            send_graph_updates(graph, stops_count, ticket, types);
        } else if (tag == recv_updates) {
            recv_graph_updates(graph, stops_count, ticket, types);
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
