#include <stdio.h>
#include "frequency.h"
#include "stop_graph.h"
#include "ticket.h"
#include "ticket_processor.h"

int main(void) {
    int fcount = 0;
    int tcount = 0;
    int scount = 0;

    StopGraph* stop = NULL;
    if (load_stop_graph_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante_guardado.csv", &stop, &scount) != 0) {
        // Fall back to expensive calculation
        if (create_stop_graph_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv", &stop, &scount) != 0) {
            printf("Error loading stop graph\n");
            return 1;
        }

        // Save result for future runs
        save_stop_graph_to_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante_guardado.csv", stop, scount);
    }

    Frequency* frequencies = NULL;
    if (calculate_frequency_from_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv", &frequencies, &fcount) != 0) {
        printf("Failed to read CSV\n");
        return 1;
    }

    Ticket* ticket = NULL;
    if (read_tickets_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\tickets_output.csv", &ticket, &tcount) != 0) {
        printf("Failed to read tickets CSV\n");
        return 1;
    }


    for (int i = 0; i < tcount; i++) {
        printf("Starting proccess of: ");
        print_ticket_line(&ticket[i]);
        process_ticket(ticket[i], stop, scount, frequencies, fcount);
        printf("Ending proccessing...\n");
        printf("\n");
        fflush(stdout);
    }

    free_tickets(ticket);
    free_stop_graph(stop);
    free_frequencies(frequencies);
    return 0;
}
