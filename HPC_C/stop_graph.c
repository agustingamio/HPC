#include "stop_graph.h"
#include <stdio.h>
#include "utils.h"
#include "planned_route.h"
int create_stop_graph_from_csv(const char* filepath, StopGraph** graph_out, int* count_out) {
    if (!filepath || !graph_out || !count_out) {
        return -1; // Error: parámetros inválidos
    }
    FILE* file = fopen(filepath, "r");
    if (!file) return -1;
    char line[MAX_LINE_LENGTH];
    int capacity = 100, count = 0;
    StopGraph* graph = malloc(capacity * sizeof(StopGraph));
    if (!graph) { fclose(file); return -1; }

    PlannedRouteCsvLine** planned_routes = NULL;
    int *planned_routes_count = 0;
    int ok = get_planned_routes_from_csv(filepath, &planned_routes, &planned_routes_count);

    if(!ok){
        return -1;
    }

    fgets(line, MAX_LINE_LENGTH, file); // skip header
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        char* parts[7];
        int i = 0;
        char* token = strtok(line, ";");
        while (token && i < 7) {
            parts[i++] = token;
            token = strtok(NULL, ";");
        }
        if (i < 7) continue;

        if (count >= capacity) {
            capacity *= 2;
            StopGraph* tmp = realloc(graph, capacity * sizeof(StopGraph));
            if (!tmp) { free(graph); fclose(file); return -1; }
            graph = tmp;
        }

        int variant_id = atoi(parts[1]);
        DayType day_type = (DayType)atoi(parts[0]);
        int stop_id = atoi(parts[3]);

        int route_count = 0;
        PlannedRouteCsvLine** routes = get_route_from_planned_routes(planned_routes, planned_routes_count, variant_id, day_type, stop_id, &route_count);

        StopGraph sg;
        sg.stop_id = stop_id;
        sg.variant_id = variant_id;
        sg.relative_stop_id = atoi(parts[4]);
        sg.day_type = day_type;

        if (sg.relative_stop_id == 1){
            sg.time_from_last_stop = 0;
        }else{
            if (routes) {
                sg.time_from_last_stop = averageTimeFromLastStop(routes, route_count); // Usar el tiempo de la ruta planificada
            } else {
                sg.time_from_last_stop = 0; // Si no hay nodo anterior, inicializar a 0
            }
        }
        sg.ticket_count = 0; // Inicializar contador de tickets
        memset(sg.last_tickets, 0, sizeof(sg.last_tickets)); // Limpiar tickets

        graph[count++] = sg;
    }
    fclose(file);
    *graph_out = graph;
    *count_out = count;
    return 0;
}

PlannedRouteCsvLine** get_route_from_planned_routes(PlannedRouteCsvLine** planned_routes, int planned_routes_count, int variant_id,int day_type,int stop_id, int routes_count){
    PlannedRouteCsvLine** routes_out = NULL;
    for(int i = 0; i< planned_routes_count; i++){
        if(planned_routes[i]->variant_id == variant_id && planned_routes[i]->day_type == day_type && planned_routes[i]->stop_id == stop_id){
            //TODO: Mem Copy
            routes_out[routes_count] = planned_routes[i];
            routes_count++;
        }
    }
    return routes_out;
}

long averageTimeFromLastStop(PlannedRouteCsvLine** routes, int route_count){
    long time_sum = 0;
    for(int i = 0; i<route_count; i++){
        time_sum += routes[i]->time_from_last_stop;
    }
    return time_sum/route_count;
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
                count++;
            }
    }

    if (count == 0){
        *result_out = NULL;
        return 0;
    }

    *result_out = malloc(count*sizeof(StopGraph*));
    if (*result_out == NULL){
        return -1;
    }

    int index = 0;
    for (int i = 0; i < graph_size; i++){
        if (graph[i].relative_stop_id < current->relative_stop_id &&
            graph[i].variant_id == current->variant_id &&
            graph[i].day_type == current->day_type){
                (*result_out)[index++] = graph[i];
            }
    }

    return count;
}

void free_stop_graph(StopGraph* graph) {
    if (graph != NULL) {
        free(graph);
    }
}

int averageTimeFromLastStop(PlannedRouteCsvLine* route, int route_count){
    int total_time = 0;
    if (route && route_count > 0) {
        for (int i = 0; i < route_count; i++) {
            total_time += route[i].time_from_last_stop;
        }
        return total_time / route_count;
    }
    return 0;
}