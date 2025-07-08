#include "ticket_processor.h"

void process_ticket(const Ticket* ticket, StopGraph* graph, int graph_count, Frequency* frequencies, int freq_count) {
    if (!ticket || !graph || graph_count <= 0 || !frequencies || freq_count <= 0) {
        return;
    }

    // Buscar la parada correspondiente en el grafo
    StopGraph* stop = NULL;
    for (int i = 0; i < graph_count; i++) {
        if (graph[i].stop_id == ticket->stop_id && graph[i].variant_id == ticket->variant_id) {
            stop = &graph[i];
            break;
        }
    }

    if (!stop) {
        return;
    }

    // Agregar el ticket a la parada
    add_ticket_to_stop(stop, ticket->sold_date);

    // TODO: Implementar la logica para actualizar el grafo de paradas
    
}