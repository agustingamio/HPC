#include "ticket.h"
#include "day_type.h"
#include "stop_graph.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#include "frequency.h"
#include "mpi_types.h"
#include "mpi_utils.h"

void predict_future_times(const Ticket ticket, int stop_graph_count, StopGraph* graph){
    StopGraph* stop = get_graph_stop_of_ticket(graph, stop_graph_count, ticket);

    StopGraph** next_stops;
    int next_stops_count = get_next_stops(graph, stop_graph_count, stop, &next_stops);

    // TODO: Check this refactor
    //calcula todos los tiempos estimados para cada parada obtenida en el grafo y las va mostrando en consola
    struct tm estimated_time = ticket.sold_date;
    for (int i = 0; i < next_stops_count; i++) {
        estimated_time = add_seconds(estimated_time, next_stops[i]->time_from_last_stop);
        next_stops[i]->next_arrival_time = estimated_time;
        if (ticket.variant_id == 0)
            printf("Estimated time for stop %d: %02d:%02d:%02d\n", next_stops[i]->relative_stop_id,
                estimated_time.tm_hour, estimated_time.tm_min, estimated_time.tm_sec);
    }
}

void message_process_predictor(StopGraph* graph, const int stops_count,
                                const MPITypes types)
{
    while (1) {
        Ticket ticket;
        MPI_Status status;

        MPI_Recv(&ticket,1,types.ticket_type,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        const enum tag_types tag = status.MPI_TAG;

        if (tag == ticket_to_process) {
            predict_future_times(ticket, stops_count, graph);
        } else if (tag == send_updates) {
            send_graph_updates(graph, stops_count, ticket, types);
        } else if (tag == recv_updates) {
            recv_graph_updates(graph, stops_count, types);
        } else if (tag == shutdown_signal) {
            break;
        }
    }
}

void main_predictor(const MPITypes types) {
    int freq_count = 0, stops_count = 0;
    Frequency* frequencies = NULL;
    StopGraph* graph = NULL;

    receive_structures(&frequencies, &freq_count, &graph, &stops_count, types);
    message_process_predictor(graph, stops_count, types);

    free_stop_graph(graph);
}