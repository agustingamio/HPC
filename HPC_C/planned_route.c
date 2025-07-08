#include "planned_route.h"

int read_planned_route_csv(const char* filepath, PlannedRouteCsvLine** lines_out, int* count_out) {
    if (!filepath || !lines_out || !count_out) {
        return -1; // Error: parámetros inválidos
    }

    // TODO: Implementar la lógica para leer el CSV y llenar las líneas planificadas
    return -2; // Error: no implementado
}

void free_planned_route(PlannedRouteCsvLine* lines) {
    if (lines != NULL) {
        free(lines);
    }
}