#include "ticket.h"
#include "day_type.h"
#include "stop_graph.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#include "frequency.h"
#include "mpi_types.h"
#include "mpi_utils.h"

int get_ticket(Ticket* ticket){
    //TODO: Ver como vamos a resolver la obtencion del ticket, de momento, datos de prueba:
    ticket->sold_date.tm_year = 125;
    ticket->sold_date.tm_mon = 9; // Octubre
    ticket->sold_date.tm_mday = 15; // 15 de Octubre
    ticket->sold_date.tm_hour = 7; // 7 AM
    ticket->sold_date.tm_min = 27; // 27 minutos
    ticket->sold_date.tm_sec = 10; // 10 segundos
    ticket->sold_date.tm_isdst = -1; // No se sabe si es horario de verano
    ticket->stop_id = 1059; // ID de la parada
    ticket->variant_id = 650; // ID de la variante
    return 1; // Retorna 1 si se obtuvo un ticket, 0 si no
}

int get_slave_by_variant(int variant_id) {
    //TODO: Implementar la logica para obtener el id del nodo esclavo que tiene la variante
    return 0;
}

StopGraph* get_stop_graph_from_slave(int slave_id, int* stop_graph_count) {
    
    *stop_graph_count = 0; // Asignar el conteo de paradas
    return NULL; // Retornar el grafo de paradas
}

void calculate_estimated_time(StopGraph stop_graph, struct tm* estimated_time) {
    long time_from_last_stop = stop_graph.time_from_last_stop;
    *estimated_time = add_seconds(*estimated_time, time_from_last_stop);
}

int predict_future_times(const Ticket ticket, int stop_graph_count, StopGraph* graph){
    //Lee la variante, tipo de dia y parada del ticket
    struct tm sold_date = ticket.sold_date;
    DayType day_type = get_day_type_from_date(sold_date);
    int stop_id = ticket.stop_id;
    int variant_id = ticket.variant_id;

    int relative_stop = get_relative_stop_by_stop_id(graph, stop_graph_count, stop_id, variant_id);
    //se queda con todas las paradas que tengan id relativo de parada superior al leifo en el ticket
    int next_stops_count = 0;
    for (int i = 0; i<stop_graph_count; i++){
        if (graph[i].relative_stop_id > relative_stop && graph[i].day_type == day_type && graph[i].variant_id == variant_id) {
            next_stops_count ++;
        }
    }

    StopGraph* next_stops[next_stops_count];
    int index = 0;
    // TODO: when stops are next to the end of the day, some will be needed to be search in the next day
    for (int i = 0; i<stop_graph_count; i++){
        if (graph[i].relative_stop_id > relative_stop && graph[i].day_type == day_type && graph[i].variant_id == variant_id) {
            next_stops[index++] = &graph[i];
        }
    }

    //calcula todos los tiempos estimados para cada parada obtenida en el grafo y las va mostrando en consola
    struct tm estimated_time = sold_date;

    for (int i = 0; i < next_stops_count; i++) {
        calculate_estimated_time(*next_stops[i], &estimated_time);
        next_stops[i]->next_arrival_time = estimated_time;
        printf("Estimated time for stop %d: %02d:%02d:%02d\n", next_stops[i]->relative_stop_id,
            estimated_time.tm_hour, estimated_time.tm_min, estimated_time.tm_sec);
        fflush(stdout);
    }

    return 0;
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
            // Prepare to receive updates
        } else if (tag == shutdown_signal) {
            break;
        }
    }
}

void receive_structures_predictor(Frequency** frequencies, int* freq_count,
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


void main_predictor(const MPITypes types) {
    int freq_count = 0, stops_count = 0;
    Frequency* frequencies = NULL;
    StopGraph* graph = NULL;

    receive_structures_predictor(&frequencies, &freq_count, &graph, &stops_count, types);
    message_process_predictor(graph, stops_count, types);

    free_stop_graph(graph);
}