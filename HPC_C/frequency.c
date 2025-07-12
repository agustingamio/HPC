#include "frequency.h"

#include <stdio.h>
#include <stdlib.h>

#include "day_type.h"
#include "planned_route.h"
#include "ticket.h"

PlannedRouteCsvLine* get_previous_departure(PlannedRouteCsvLine* actual_departure, PlannedRouteCsvLine* departures, const int departures_count) {
    PlannedRouteCsvLine* previous = NULL;
    long max_freq = -1;

    // 1. Mismo VariantId, DayType, frecuencia menor
    for (int i = 0; i < departures_count; i++) {
        if (departures[i].variant_id == actual_departure->variant_id &&
            departures[i].day_type == actual_departure->day_type) {

            const long freq_secs = departures[i].frequency.tm_hour * 3600 +
                             departures[i].frequency.tm_min * 60 +
                             departures[i].frequency.tm_sec;

            const long actual_freq_secs = actual_departure->frequency.tm_hour * 3600 +
                                    actual_departure->frequency.tm_min * 60 +
                                    actual_departure->frequency.tm_sec;

            if (freq_secs < actual_freq_secs && freq_secs > max_freq) {
                max_freq = freq_secs;
                previous = &departures[i];
            }
        }
    }

    if (previous != NULL) return previous;

    // 2. Si es WORKING_DAY, devuelvo última frecuencia de ese día
    if (actual_departure->day_type == WORKING_DAY) {
        for (int i = 0; i < departures_count; i++) {
            if (departures[i].variant_id == actual_departure->variant_id &&
                departures[i].day_type == WORKING_DAY) {

                const long freq_secs = departures[i].frequency.tm_hour * 3600 +
                                 departures[i].frequency.tm_min * 60 +
                                 departures[i].frequency.tm_sec;

                if (freq_secs > max_freq) {
                    max_freq = freq_secs;
                    previous = &departures[i];
                }
            }
        }
        return previous;
    }

    // 3. Día anterior
    const DayType prev_day = get_previous_day_type(actual_departure->day_type);
    for (int i = 0; i < departures_count; i++) {
        if (departures[i].variant_id == actual_departure->variant_id &&
            departures[i].day_type == prev_day) {

            const long freq_secs = departures[i].frequency.tm_hour * 3600 +
                             departures[i].frequency.tm_min * 60 +
                             departures[i].frequency.tm_sec;

            if (freq_secs > max_freq) {
                max_freq = freq_secs;
                previous = &departures[i];
            }
        }
    }

    if (previous != NULL) return previous;

    // 4. Día anterior al anterior
    const DayType prev_prev_day = get_previous_day_type(prev_day);
    for (int i = 0; i < departures_count; i++) {
        if (departures[i].variant_id == actual_departure->variant_id &&
            departures[i].day_type == prev_prev_day) {

            const long freq_secs = departures[i].frequency.tm_hour * 3600 +
                             departures[i].frequency.tm_min * 60 +
                             departures[i].frequency.tm_sec;

            if (freq_secs > max_freq) {
                max_freq = freq_secs;
                previous = &departures[i];
            }
        }
    }

    // 5. Si no se encuentra nada, devuelvo el actual
    return previous != NULL ? previous : actual_departure;
}

PlannedRouteCsvLine* get_next_departure(PlannedRouteCsvLine* actual_departure, PlannedRouteCsvLine* departures, const int departures_count) {
    PlannedRouteCsvLine* next = NULL;
    long min_freq = LONG_MAX;

    // 1. Mismo VariantId, DayType, frecuencia mayor
    for (int i = 0; i < departures_count; i++) {
        if (departures[i].variant_id == actual_departure->variant_id &&
            departures[i].day_type == actual_departure->day_type) {

            const long freq_secs = departures[i].frequency.tm_hour * 3600 +
                             departures[i].frequency.tm_min * 60 +
                             departures[i].frequency.tm_sec;

            const long actual_freq_secs = actual_departure->frequency.tm_hour * 3600 +
                                    actual_departure->frequency.tm_min * 60 +
                                    actual_departure->frequency.tm_sec;

            if (freq_secs > actual_freq_secs && freq_secs < min_freq) {
                min_freq = freq_secs;
                next = &departures[i];
            }
        }
    }

    if (next != NULL) return next;

    // 2. Si es WORKING_DAY, tomo la primer frecuencia del WORKING_DAY
    if (actual_departure->day_type == WORKING_DAY) {
        min_freq = LONG_MAX;
        for (int i = 0; i < departures_count; i++) {
            if (departures[i].variant_id == actual_departure->variant_id &&
                departures[i].day_type == WORKING_DAY) {

                const long freq_secs = departures[i].frequency.tm_hour * 3600 +
                                 departures[i].frequency.tm_min * 60 +
                                 departures[i].frequency.tm_sec;

                if (freq_secs < min_freq) {
                    min_freq = freq_secs;
                    next = &departures[i];
                }
            }
        }
        return next;
    }

    // 3. Día siguiente
    const DayType next_day = get_next_day_type(actual_departure->day_type);
    min_freq = LONG_MAX;
    for (int i = 0; i < departures_count; i++) {
        if (departures[i].variant_id == actual_departure->variant_id &&
            departures[i].day_type == next_day) {

            const long freq_secs = departures[i].frequency.tm_hour * 3600 +
                             departures[i].frequency.tm_min * 60 +
                             departures[i].frequency.tm_sec;

            if (freq_secs < min_freq) {
                min_freq = freq_secs;
                next = &departures[i];
            }
        }
    }

    if (next != NULL) return next;

    // 4. Día siguiente al siguiente
    const DayType next_next_day = get_next_day_type(next_day);
    min_freq = LONG_MAX;
    for (int i = 0; i < departures_count; i++) {
        if (departures[i].variant_id == actual_departure->variant_id &&
            departures[i].day_type == next_next_day) {

            const long freq_secs = departures[i].frequency.tm_hour * 3600 +
                             departures[i].frequency.tm_min * 60 +
                             departures[i].frequency.tm_sec;

            if (freq_secs < min_freq) {
                min_freq = freq_secs;
                next = &departures[i];
            }
        }
    }

    return next != NULL ? next : actual_departure;
}

time_t create_base_time(const struct tm time_info) {
    struct tm base = {0};
    base.tm_year = 2000 - 1900;
    base.tm_mon = 0;
    base.tm_mday = 1;
    base.tm_hour = time_info.tm_hour;
    base.tm_min = time_info.tm_min;
    base.tm_sec = time_info.tm_sec;
    return mktime(&base);
}

long calculate_frequency_average(const PlannedRouteCsvLine* prev, const PlannedRouteCsvLine* actual, const PlannedRouteCsvLine* next) {
    time_t prev_time = create_base_time(prev->frequency);
    const time_t actual_time = create_base_time(actual->frequency);
    time_t next_time = create_base_time(next->frequency);

    // Ajustar día hacia atrás para previous si cambia DayType
    const DayType actual_day_type = actual->day_type;
    DayType prev_day_type = prev->day_type;
    while (prev_day_type != actual_day_type) {
        prev_time -= 86400; // Restar 1 día
        prev_day_type = get_next_day_type(prev_day_type);
    }

    // Ajustar día hacia adelante para next si cambia DayType
    DayType next_day_type = next->day_type;
    while (next_day_type != actual_day_type) {
        next_time += 86400; // Sumar 1 día
        next_day_type = get_previous_day_type(next_day_type);
    }

    // Calcular diferencias de tiempo en segundos
    const double diff_prev = difftime(actual_time, prev_time);
    const double diff_next = difftime(next_time, actual_time);

    // Asegurar diferencias positivas
    const long diff_prev_sec = diff_prev > 0 ? diff_prev : LONG_MAX;
    const long diff_next_sec = diff_next > 0 ? diff_next : LONG_MAX;

    // Devolver promedio del mínimo intervalo
    const long min_ticks = diff_prev_sec < diff_next_sec ? diff_prev_sec : diff_next_sec;
    return min_ticks / 2;
}

int calculate_frequency_from_csv(const char* filepath, Frequency** frequencies_out, int* count_out) {
    *count_out = 0;
    PlannedRouteCsvLine* departures = NULL;
    if (get_departures(filepath, &departures, count_out) != 0) {
        printf("Failed to get departure routes\n");
        return 1;
    }

    *frequencies_out = malloc(*count_out * sizeof(Frequency));
    for (int i = 0; i < *count_out; i++) {
        PlannedRouteCsvLine* actual_departure = &departures[i];
        PlannedRouteCsvLine* previous_departure = get_previous_departure(actual_departure, departures, *count_out);
        PlannedRouteCsvLine* next_departure = get_next_departure(actual_departure, departures, *count_out);

        long avg_seconds = calculate_frequency_average(previous_departure, actual_departure, next_departure);

        if (avg_seconds > 86400) {  // More than 24 hours
            int count = 0;
            for (int j = 0; j < *count_out; j++) {
                if ((*frequencies_out)[j].variant_id == actual_departure->variant_id) {
                    count++;
                }
            }
            if (count > 1) {
                avg_seconds = 5 * 60; // Default to 5 minutes
            }
        }

        (*frequencies_out)[i].variant_id = actual_departure->variant_id;
        (*frequencies_out)[i].day_type = actual_departure->day_type;
        (*frequencies_out)[i].departure_time = actual_departure->hour;
        (*frequencies_out)[i].frequency_average = avg_seconds;
    }

    return 0;
}

long get_frequency_average(const Frequency* frequencies, const int frequencies_count, const Ticket ticket, const struct tm estimated_datetime_of_departure) {
    const DayType day_type = get_day_type_from_date(estimated_datetime_of_departure);

    const Frequency* best_match = NULL;
    long min_diff = LONG_MAX;

    const long estimated_seconds = estimated_datetime_of_departure.tm_hour * 3600 +
                             estimated_datetime_of_departure.tm_min * 60 +
                             estimated_datetime_of_departure.tm_sec;

    for (int i = 0; i < frequencies_count; i++) {
        if (frequencies[i].variant_id == ticket.variant_id &&
            frequencies[i].day_type == day_type) {

            long freq_seconds = frequencies[i].departure_time.tm_hour * 3600 +
                                frequencies[i].departure_time.tm_min * 60 +
                                frequencies[i].departure_time.tm_sec;

            long diff = labs(freq_seconds - estimated_seconds);  // absolute difference

            if (diff < min_diff) {
                min_diff = diff;
                best_match = &frequencies[i];
            }
        }
    }

    return (best_match != NULL) ? best_match->frequency_average : -1;
}

void print_frequency_line(const Frequency* freq) {
    char dep_time_str[9];
    strftime(dep_time_str, sizeof(dep_time_str), "%H:%M:%S", &freq->departure_time);
    printf("Variant ID: %d, Day Type: %d, Departure Time: %s, Frequency Average: %ld seconds\n",
           freq->variant_id, freq->day_type, dep_time_str, freq->frequency_average);
}

void free_frequencies(Frequency* frequencies) {
    if (frequencies != NULL) {
        free(frequencies);
    }
}