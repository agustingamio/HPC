#include "ticket_processor.h"

#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

int get_stops_to_update(
    StopGraph* stops, int stop_count,
    const long frequency_secs, Ticket actual_ticket,
    StopGraph** stops_to_update_out, int* stops_to_update_count,
    struct tm* matched_ticket_out
) {

    StopGraph** result = malloc(stop_count * sizeof(StopGraph*));
    if (!result) return 1;

    int result_count = 0;
    result[result_count++] = &stops[0];

    long estimated_commute_time = stops[0].time_from_last_stop;

    for (int i = 1; i < stop_count; i++) {
        StopGraph* stop = &stops[i];
        result[result_count++] = stop;

        if (stop->relative_stop_id == 1) continue;

        for (int j = 0; j < stop->ticket_count; j++) {
            struct tm ticket_tm = stop->last_tickets[j];

            const time_t ticket_time = mktime(&ticket_tm);
            const time_t sold_time = mktime(&actual_ticket.sold_date);
            const time_t actual_ticket_adjusted = sold_time - estimated_commute_time;

            time_t lower_bound, upper_bound;
            if (frequency_secs > 864000) { // > 10 days
                lower_bound = upper_bound = 0;
            } else {
                lower_bound = ticket_time - frequency_secs;
                upper_bound = ticket_time + frequency_secs;
            }

            if (actual_ticket_adjusted > lower_bound && actual_ticket_adjusted < upper_bound) {
                // Found matching ticket
                *stops_to_update_out = malloc(result_count * sizeof(StopGraph*));
                if (!*stops_to_update_out) {
                    free(result);
                    return 1;
                }
                memcpy(*stops_to_update_out, result, result_count * sizeof(StopGraph*));
                *stops_to_update_count = result_count;
                *matched_ticket_out = ticket_tm;
                free(result);
                return 0;
            }
        }

        estimated_commute_time += stop->time_from_last_stop;
    }

    // No match found
    *stops_to_update_out = result;
    *stops_to_update_count = result_count;
    return 1;
}

void process_ticket(Ticket ticket, StopGraph* graph, int graph_count, Frequency* frequencies, int freq_count) {
    StopGraph* stop = NULL;
    for (int i = 0; i < graph_count; i++) {
        if (graph[i].stop_id == ticket.stop_id &&
            graph[i].variant_id == ticket.variant_id &&
            graph[i].day_type == get_day_type_from_date(ticket.sold_date)) {
            stop = &graph[i];
            break;
        }
    }

    if (stop == NULL) return;

    // Comprobar si el ticket ya fue agregado recientemente
    int already_added = 0;
    for (int i = 0; i < stop->ticket_count; i++) {
        double minutes = difftime(mktime(&ticket.sold_date), mktime(&stop->last_tickets[i])) / 60.0;
        if (fabs(minutes) < 1.0) {
            already_added = 1;
            break;
        }
    }
    if (!already_added) {
        add_ticket_to_stop(stop, ticket.sold_date);
    }

    if (stop->relative_stop_id == 1) return;

    StopGraph* previous_stops = NULL;
    int previous_stops_count = get_previous_stops(graph, graph_count, stop, &previous_stops);

    // Calcular hora estimada de partida restando los tiempos acumulados
    time_t estimated_departure = mktime(&ticket.sold_date);
    for (int i = 0; i < previous_stops_count; i++) {
        estimated_departure -= previous_stops[i].time_from_last_stop;
    }
    const struct tm* estimated_tm = localtime(&estimated_departure);

    long range = get_frequency_average(frequencies, freq_count, ticket, *estimated_tm);

    // Obtener stops a actualizar
    StopGraph* stops_to_update = NULL;
    int update_count = 0;
    struct tm* last_sold_ticket_time = NULL;
    get_stops_to_update(previous_stops, previous_stops_count, range, ticket, &stops_to_update, &update_count, last_sold_ticket_time);

    if (last_sold_ticket_time == NULL) return;

    // Tiempo desde el último ticket vendido
    double time_from_last_sold_ticket = difftime(mktime(&ticket.sold_date), mktime(last_sold_ticket_time));

    // Suma teórica de los tiempos entre paradas
    long theoretical_time = 0;
    for (int i = 1; i < update_count; i++) {
        theoretical_time += stops_to_update[i].time_from_last_stop;
    }

    // Actualizar tiempos
    for (int i = 1; i < update_count; i++) {
        double percentage = 100.0;
        if (theoretical_time != 0) {
            percentage = (double)stops_to_update[i].time_from_last_stop * 100.0 / theoretical_time;
        }
        double time_to_add = time_from_last_sold_ticket * percentage / 100.0;
        if (isnan(time_to_add)) time_to_add = 0;
        double updated = ((double)stops_to_update[i].time_from_last_stop + time_to_add) / 2.0;
        stops_to_update[i].time_from_last_stop = (long)updated;
    }
}