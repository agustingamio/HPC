#ifndef FREQUENCY_H
#define FREQUENCY_H

#include <time.h>
#include "planned_route.h"
#include "ticket.h"

typedef struct {
    int variant_id;
    DayType day_type;
    struct tm departure_time;
    long frequency_average; // en segundos
} Frequency;

// Calcula la frecuencia promedio a partir del CSV
int calculate_frequency_from_csv(const char* filepath, Frequency** frequencies_out, int* count_out);

// Obtiene la frecuencia promedio más cercana al ticket
long get_frequency_average(Frequency* frequencies, int freq_count, Ticket ticket, struct tm estimated_departure);

// Printea la frequencia
void print_frequency_line(const Frequency* freq);

//Libera la memoria de las frecuencias
void free_frequencies(Frequency* frequencies);

#endif