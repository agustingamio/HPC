#ifndef TICKET_PROCESSOR_H
#define TICKET_PROCESSOR_H

#include "ticket.h"
#include "stop_graph.h"
#include "frequency.h"

// Procesa un ticket, actualizando la estructura del grafo
void process_ticket(Ticket ticket, StopGraph* graph, int graph_count, Frequency* frequencies, int freq_count);

#endif