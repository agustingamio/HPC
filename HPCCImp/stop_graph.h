#ifndef STOP_GRAPH_H
#define STOP_GRAPH_H

#include <time.h>
#include "day_type.h"

#define MAX_TICKETS_PER_STOP 10

typedef struct {
    int stop_id;
    int variant_id;
    int relative_stop_id;
    DayType day_type;
    long time_from_last_stop; // segundos
    struct tm last_tickets[MAX_TICKETS_PER_STOP];
    int ticket_count;
} StopGraph;

// Crea la estructura de gráfo a partir de CSV
int create_stop_graph_from_csv(const char* file_path, StopGraph** out_graph, int* out_graph_count);

// Agrega un ticket a la lista reciente del nodo (manteniendo máximo 10)
void add_ticket_to_stop(StopGraph* stop, struct tm ticket_time);

// Devuelve el stop anterior en la ruta
StopGraph* get_previous_stop(StopGraph* graph, int graph_size, StopGraph* current);

// Devuelve los stops anteriores en la ruta
int get_previous_stops(StopGraph* graph, int graph_size, StopGraph* current, StopGraph** result_out);

void print_stop_graph_line(const StopGraph* stop);

void free_stop_graph(StopGraph* tickets);

#endif
