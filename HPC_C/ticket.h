#ifndef TICKET_H
#define TICKET_H

#include <time.h>

typedef struct {
    struct tm sold_date;
    int stop_id;
    int variant_id;
} Ticket;

// Lee y parsea el CSV de tickets
int read_tickets_csv(const char* filepath, Ticket** tickets_out, int* count_out);
// Libera la memoria de los tickets
void free_tickets(Ticket* tickets);
#endif
