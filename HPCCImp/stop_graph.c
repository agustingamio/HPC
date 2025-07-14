#include "stop_graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "planned_route.h"

int create_stop_graph_from_csv(const char* file_path, StopGraph** out_graph, int* out_graph_count) {
    PlannedRouteCsvLine* routes = NULL;
    int route_count = 0;

    if (get_planned_routes_from_csv(file_path, &routes, &route_count) != 0) {
        return -1;
    }

    typedef struct {
        int variant_id;
        DayType day_type;
        int stop_id;
    } StopGraphGroupKey;

    typedef struct {
        StopGraphGroupKey key;
        double total_time;
        int count;
        int relative_stop_id;
    } TempStopGraphGroup;

    int group_capacity = 100;
    int group_count = 0;
    TempStopGraphGroup* groups = malloc(sizeof(TempStopGraphGroup) * group_capacity);
    if (!groups) {
        free(routes);
        return -1;
    }

    for (int i = 0; i < route_count; i++) {
        PlannedRouteCsvLine* route = &routes[i];

        StopGraphGroupKey key = {
            .variant_id = route->variant_id,
            .day_type = route->day_type,
            .stop_id = route->stop_id
        };

        int found = 0;
        for (int j = 0; j < group_count; j++) {
            if (groups[j].key.variant_id == key.variant_id &&
                groups[j].key.day_type == key.day_type &&
                groups[j].key.stop_id == key.stop_id) {
                groups[j].total_time += route->time_from_last_stop;
                groups[j].count += 1;
                found = 1;
                break;
            }
        }

        if (!found) {
            if (group_count >= group_capacity) {
                group_capacity *= 2;
                TempStopGraphGroup* temp = realloc(groups, sizeof(TempStopGraphGroup) * group_capacity);
                if (!temp) {
                    free(routes);
                    free(groups);
                    return -1;
                }
                groups = temp;
            }

            groups[group_count].key = key;
            groups[group_count].total_time = route->time_from_last_stop;
            groups[group_count].count = 1;
            groups[group_count].relative_stop_id = route->relative_stop_id;
            group_count++;
        }
    }

    StopGraph* graph = malloc(sizeof(StopGraph) * group_count);
    if (!graph) {
        free(routes);
        free(groups);
        return -1;
    }

    for (int i = 0; i < group_count; i++) {
        graph[i].stop_id = groups[i].key.stop_id;
        graph[i].variant_id = groups[i].key.variant_id;
        graph[i].day_type = groups[i].key.day_type;
        graph[i].relative_stop_id = groups[i].relative_stop_id;
        graph[i].time_from_last_stop = (long)(groups[i].total_time / groups[i].count);
        graph[i].ticket_count = 0;
        memset(graph[i].last_tickets, 0, sizeof(graph[i].last_tickets));
    }

    *out_graph = graph;
    *out_graph_count = group_count;

    free(groups);
    free(routes);
    return 0;
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

void print_stop_graph_line(const StopGraph* stop) {
    if (stop) {
        printf("Stop ID: %d, Variant ID: %d, Day Type: %d, Relative Stop ID: %d, Time from Last Stop: %ld seconds, Ticket Count: %d\n",
               stop->stop_id, stop->variant_id, stop->day_type, stop->relative_stop_id,
               stop->time_from_last_stop, stop->ticket_count);
    }
}

void free_stop_graph(StopGraph* graph) {
    if (graph != NULL) {
        free(graph);
    }
}