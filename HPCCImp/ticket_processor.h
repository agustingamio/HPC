#ifndef TICKET_PROCESSOR_H
#define TICKET_PROCESSOR_H

#include "ticket.h"
#include "stop_graph.h"
#include "frequency.h"

// Procesa un ticket, actualizando la estructura del grafo
void process_ticket(const Ticket ticket, StopGraph* graph, const int graph_count, const Frequency* frequencies, const int freq_count);

#endif