#include "stop_graph.h"

int create_stop_graph_from_csv(const char* filepath, StopGraph** graph_out, int* count_out) {
    if (!filepath || !graph_out || !count_out) {
        return -1; // Error: parámetros inválidos
    }

    // TODO: implementar la lógica para leer el CSV y llenar el grafo
    return -2; // Error: no implementado
}

void add_ticket_to_stop(StopGraph* stop, struct tm ticket_time) {
    if (!stop || stop->ticket_count >= MAX_TICKETS_PER_STOP) {
        return; // Error: parada inválida o máximo de tickets alcanzado
    }

    // Agregar el ticket a la lista reciente
    stop->last_tickets[stop->ticket_count] = ticket_time;
    stop->ticket_count++;
}

StopGraph* get_previous_stop(StopGraph* graph, int graph_size, StopGraph* current) {
    if (!graph || graph_size <= 0 || !current) {
        return NULL; // Error: parámetros inválidos
    }

    for (int i = 0; i < graph_size; i++) {
        if (graph[i].relative_stop_id == current->relative_stop_id - 1 &&
            graph[i].variant_id == current->variant_id &&
            graph[i].day_type == current->day_type) {
            return &graph[i]; // Encontrado el stop anterior
            }
    }

    return NULL; // No se encontró el stop anterior
}
int get_previous_stops(StopGraph* graph, int graph_size, StopGraph* current, StopGraph** result_out) {
    if (!graph || graph_size <= 0 || !current || !result_out) {
        return -1; // Error: parámetros inválidos
    }

    int count = 0;
    for (int i = 0; i < graph_size; i++) {
        if (graph[i].relative_stop_id < current->relative_stop_id &&
            graph[i].variant_id == current->variant_id &&
            graph[i].day_type == current->day_type) {
            result_out[count++] = &graph[i];
            }
    }

    return count;
}

void free_stop_graph(StopGraph* graph) {
    if (graph != NULL) {
        free(graph);
    }
}