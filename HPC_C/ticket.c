#include "ticket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_tickets_csv(const char* filepath, Ticket** tickets_out, int* count_out) {

    //TODO: Revisar esto, ya que el csv en realidad tiene mas campos que lo que guardamos en el ticket, 
    // por ende al leerlos, va a haber que parsear mas y guardar solo lo que nos interesa
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("No se pudo abrir el archivo");
        return -1;
    }

    int count = 0;
    int capacity = 10;
    Ticket* tickets = (Ticket*) malloc(capacity * sizeof(Ticket));
    if (!tickets) {
        fclose(file);
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (count >= capacity) {
            capacity += 10;
            Ticket* temp = (Ticket*) realloc(tickets, capacity * sizeof(Ticket));
            if (!temp) {
                free(tickets);
                fclose(file);
                return -1;
            }
            tickets = temp;
        }

        int sold_date, stop_id, variant_id;
        if (sscanf(line, "%d,%d,%d", &sold_date, &stop_id, &variant_id) == 3) {
            tickets[count].sold_date = sold_date;
            tickets[count].stop_id = stop_id;
            tickets[count].variant_id = variant_id;
            count++;
        } else {
            fprintf(stderr, "Linea invalida: %s\n", line);
        }
    }

    fclose(file);
    *tickets_out = tickets;
    *count_out = count;
    return 0;
}

void free_tickets(Ticket* tickets) {
    if (tickets != NULL) {
        free(tickets);
    }
}
