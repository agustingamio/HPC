#ifndef UTILS_H
#define UTILS_H

#include <time.h>

// Devuelve la diferencia en segundos entre dos struct tm (solo parte horaria)
long time_diff_in_seconds(struct tm a, struct tm b);

// Suma una cantidad de segundos a un struct tm
struct tm add_seconds(struct tm time, long seconds);

// Compara dos horas (ignora la fecha), devuelve diferencia en segundos
long compare_times(struct tm a, struct tm b);

// Retorna 1 si ambos struct tm tienen la misma hora (ignora la fecha)
int same_time(struct tm a, struct tm b);

#endif
