#include <stdio.h>
#include "frequency.h"
#include "stop_graph.h"
#include "ticket.h"
#include "ticket_processor.h"

int main(void) {
    int fcount = 0;
    int tcount = 0;
    int scount = 0;

    Ticket* ticket = NULL;
    if (read_tickets_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\viajes_stm_042025.csv", &ticket, &tcount) != 0) {
        printf("Failed to read tickets CSV\n");

        return 1;
    }

    StopGraph* stop = NULL;
    if (create_stop_graph_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv", &stop, &scount) != 0) {
        printf("Failed to read stop graph CSV\n");
        return 1;
    }

    Frequency* frequencies = NULL;
    if (calculate_frequency_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv", &frequencies, &fcount) != 0) {
        printf("Failed to read CSV\n");
        free_stop_graph(stop);
        return 1;
    }



    for (int i = 0; i < fcount; i++) {
        process_ticket(ticket[i], stop, scount, frequencies, fcount);
    }

    free_frequencies(frequencies);
    free_stop_graph(stop);
    free_tickets(ticket);
    return 0;
}
