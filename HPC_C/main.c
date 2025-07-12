#include <stdio.h>
#include "frequency.h"
#include "ticket.h"

int main(void) {
    int fcount = 0;
    int tcount = 0;

    Frequency* frequencies = NULL;
    if (calculate_frequency_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv", &frequencies, &fcount) != 0) {
        printf("Failed to read CSV\n");
        return 1;
    }

    Ticket* ticket = NULL;
    if (read_tickets_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\viajes_stm_042025.csv", &ticket, &tcount) != 0) {
        printf("Failed to read tickets CSV\n");
        free_frequencies(frequencies);
        return 1;
    }

    for (int i = 0; i < tcount; i++) {
        print_ticket_line(&ticket[i]);
    }

    for (int i = 0; i < fcount; i++) {
        print_frequency_line(&frequencies[i]);
    }

    free_frequencies(frequencies);
    free_tickets(ticket);
    return 0;
}