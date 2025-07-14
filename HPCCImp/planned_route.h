#ifndef PLANNED_ROUTE_H
#define PLANNED_ROUTE_H

#include <time.h>
#include "day_type.h"

typedef struct {
    DayType day_type;
    int variant_id;
    struct tm frequency;     // Usado como TimeOnly
    int stop_id;
    int relative_stop_id;
    struct tm hour;          // También como TimeOnly
    char last_day;           // 'N', 'S', '*'
    struct tm departure_time; // Solo si RelativeStopId == 1, puede ser nulo
    int has_departure_time; // ?
    long time_from_last_stop; // segundos, puede ser -1 para null
} PlannedRouteCsvLine;

// Lee y parsea el CSV de rutas planificadas
// Devuelve 0 en éxito, -1 en error
int read_planned_route_csv(const char* filepath, PlannedRouteCsvLine** lines_out, int* count_out);

int get_departures(const char* filepath, PlannedRouteCsvLine** departures, int* departures_count);

int get_planned_routes_from_csv(const char* filepath, PlannedRouteCsvLine** output, int* count_out);

// Print the PlannedRouteCsvLine for debugging
void print_planned_route_line(const PlannedRouteCsvLine* line);

// Libera la memoria de las líneas planificadas
void free_planned_route(PlannedRouteCsvLine* lines);
#endif
