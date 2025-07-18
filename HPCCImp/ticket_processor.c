#include "ticket_processor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

int get_stops_to_update(
    StopGraph** stops, int stop_count,
    const long frequency_secs, Ticket actual_ticket,
    StopGraph*** stops_to_update_out, int* stops_to_update_count,
    struct tm* matched_ticket_out
) {

    *stops_to_update_out = malloc(stop_count * sizeof(StopGraph*));
    if (!*stops_to_update_out) return 1;

    *stops_to_update_count = 0;
    (*stops_to_update_out)[(*stops_to_update_count)++] = stops[0];

    long estimated_commute_time = stops[0]->time_from_last_stop;

    for (int i = 1; i < stop_count; i++) {
        StopGraph* stop = stops[i];
        (*stops_to_update_out)[(*stops_to_update_count)++] = stop;

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
                *matched_ticket_out = ticket_tm;
                return 0;
            }
        }

        estimated_commute_time += stop->time_from_last_stop;
    }

    // No match found
    return 1;
}

void process_ticket(const Ticket ticket, StopGraph* graph, const int graph_count, const Frequency* frequencies, const int freq_count) {
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
    print_stop_graph_line(stop);

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
        printf("Ticket added to stop\n");
    } else {
        printf("Ticket NOT added to stop\n");
    }

    if (stop->relative_stop_id == 1) return;

    StopGraph** previous_stops = NULL;
    int previous_stops_count = get_previous_stops(graph, graph_count, stop, &previous_stops);
    printf("Previous stops count: %d\n", previous_stops_count);

    // Calcular hora estimada de partida restando los tiempos acumulados
    time_t estimated_departure = mktime(&ticket.sold_date);
    for (int i = 0; i < previous_stops_count; i++) {
        estimated_departure -= previous_stops[i]->time_from_last_stop;
    }
    const struct tm* estimated_tm = localtime(&estimated_departure);
    printf("Estimated departure time: %02d:%02d:%02d\n",
           estimated_tm->tm_hour, estimated_tm->tm_min, estimated_tm->tm_sec);

    long range = get_frequency_average(frequencies, freq_count, ticket, *estimated_tm);
    printf("Frequency range: %ld seconds\n", range);

    // Obtener stops a actualizar
    StopGraph** stops_to_update = NULL;
    int update_count = 0;
    struct tm last_sold_ticket_time;
    int found_ticket = get_stops_to_update(previous_stops, previous_stops_count, range, ticket, &stops_to_update, &update_count, &last_sold_ticket_time);
    printf("Stops to update count: %d\n", update_count);
    if (found_ticket == 0) {
        printf("Ticket found for update\n");
    } else {
        printf("No matching ticket found\n");
    }

    if (found_ticket == 1) return;

    // Tiempo desde el último ticket vendido
    double time_from_last_sold_ticket = difftime(mktime(&ticket.sold_date), mktime(&last_sold_ticket_time));
    printf("Time from last sold ticket: %.2f seconds\n", time_from_last_sold_ticket);

    // Suma teórica de los tiempos entre paradas
    long theoretical_time = 0;
    for (int i = 1; i < update_count; i++) {
        theoretical_time += stops_to_update[i]->time_from_last_stop;
    }

    printf("Theoretical time: %ld seconds\n", theoretical_time);

    // Actualizar tiempos
    for (int i = 1; i < update_count; i++) {
        double percentage = 100.0;
        if (theoretical_time != 0) {
            percentage = (double)stops_to_update[i]->time_from_last_stop * 100.0 / theoretical_time;
        }
        double time_to_add = time_from_last_sold_ticket * percentage / 100.0;
        if (isnan(time_to_add)) time_to_add = 0;
        double updated = ((double)stops_to_update[i]->time_from_last_stop + time_to_add) / 2.0;
        stops_to_update[i]->time_from_last_stop = (long)updated;
        printf("Updated time for stop %d: %ld seconds\n", stops_to_update[i]->stop_id, stops_to_update[i]->time_from_last_stop);
    }
}