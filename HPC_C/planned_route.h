#ifndef PLANNED_ROUTE_H
#define PLANNED_ROUTE_H

#include <time.h>
#include "day_type.h"

typedef struct {
    int variant_id;
    int stop_id;
    DayType day_type;
    struct tm frequency;     // Usado como TimeOnly
    struct tm hour;          // También como TimeOnly
    int relative_stop_id;
    char last_day;           // 'N', 'S', '*'
    struct tm departure_time; // Solo si RelativeStopId == 1, puede ser nulo
    int has_departure_time;
    long time_from_last_stop; // segundos, puede ser -1 para null
} PlannedRouteCsvLine;

// Lee y parsea el CSV de rutas planificadas
int read_planned_route_csv(const char* filepath, PlannedRouteCsvLine** lines_out, int* count_out);

// Libera la memoria de las líneas planificadas
void free_planned_route(PlannedRouteCsvLine* lines);
#endif
