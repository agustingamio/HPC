#include "frequency.h"

int calculate_frequency_from_csv(const char* filepath, Frequency** frequencies_out, int* count_out) {
    if (!filepath || !frequencies_out || !count_out) {
        return -1; // Error: parámetros inválidos
    }

    // TODO: Implementar la lógica para leer el CSV y llenar las frecuencias
    return -2; // Error: no implementado
}

long get_frequency_average(Frequency* frequencies, int freq_count, Ticket ticket, struct tm estimated_departure) {
    if (!frequencies || freq_count <= 0) {
        return -1; // Error: parámetros inválidos
    }

    for (int i = 0; i < freq_count; i++) {
        if (frequencies[i].variant_id == ticket.variant_id && 
            frequencies[i].day_type == get_day_type_from_date(ticket.sold_date)) {
            // Aca se podría hacer una comparación más compleja con estimated_departure si es necesario
            return frequencies[i].frequency_average;
        }
    }

    return -1; // No se encontró una frecuencia adecuada
}

void free_frequencies(Frequency* frequencies) {
    if (frequencies != NULL) {
        free(frequencies);
    }
}