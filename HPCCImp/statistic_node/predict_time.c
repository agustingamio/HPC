
#include <../ticket.h>
#include <../day_type.h>
#include <../stop_graph.h>
#include <../utils.h>

int get_ticket(Ticket* ticket){
    // Ver como vamos a resolver la obtencion del ticket
    ticket->sold_date = ;
    ticket->stop_id = ;
    ticket->variant_id = ;
}

int get_slave_by_variant(int variant_id) {
    // Implementar la logica para obtener el id del nodo esclavo que tiene la variante
    return 0;
}

StopGraph** get_stop_graph_from_slave(int slave_id, int* stop_graph_count) {
    // Implementar la logica para obtener el grafo de paradas del nodo esclavo
    *stop_graph_count = 0; // Asignar el conteo de paradas
    return NULL; // Retornar el grafo de paradas
}

void calculate_estimated_time(StopGraph* stop_graph, struct tm sold_date, struct tm* estimated_time) {
    // Calcula el tiempo estimado para la parada
    long time_from_last_stop = stop_graph->time_from_last_stop;
    *estimated_time = add_seconds(sold_date, time_from_last_stop);
}

void predict_time(){
// Descomentar para cuando esten las funciones finales.
// while(1){
    //recibe un ticket
    Ticket ticket;
    int found = get_ticket(&ticket);
    if(found){
        //Lee la variante, tipo de dia y parada del ticket
        struct tm sold_date;
        DayType day_type = get_day_type_from_date(ticket.sold_date);
        int stop_id = ticket.stop_id;
        int variant_id = ticket.variant_id;

        // Le pregunta al nodo maestro quien tiene esa variante.
        int slave_id = get_slave_by_variant(variant_id);

        // Le pide al nodo, que le mande el grafo que tiene
        int stop_graph_count = 0;
        StopGraph** graph = get_stop_graph_from_slave(slave_id, &stop_graph_count);
        int relative_stop = get_relative_stop_by_stop_id(graph, stop_graph_count, stop_id, variant_id);
        //se queda con todas las paradas que tengan id relativo de parada superior al leifo en el ticket
        int next_stops_count = 0;
        for (int i = 0; i<stop_graph_count; i++){
            if (graph[i]->relative_stop_id > relative_stop && graph[i]->day_type == day_type && graph[i]->variant_id == variant_id) {
                next_stops_count ++;
            }
        }

        StopGraph** next_stops = malloc(next_stops_count * sizeof(StopGraph*));
        if (!next_stops) {
            fprintf(stderr, "Error allocating memory for next stops\n");
            return -1;
        }

        for (int i = 0; i<next_stops_count; i++){
            if (next_stops[i]->relative_stop_id > relative_stop && next_stops[i]->day_type == day_type && next_stops[i]->variant_id == variant_id) {
                next_stops[i] = graph[i];
            }
        }

        //calcula todos los tiempos estimados para cada parada obtenida en el grafo y las va mostrando en consola
        for (int i = 0; i < next_stops_count; i++) {
            struct tm* estimated_time = malloc(sizeof(struct tm));
            if (!estimated_time) {
                fprintf(stderr, "Error allocating memory for estimated_time\n");
                continue;
            }
            calculate_estimated_time(next_stops[i], sold_date, estimated_time);
            printf("Estimated time for stop %d: %02d:%02d:%02d\n", next_stops[i]->relative_stop_id, 
                estimated_time->tm_hour, estimated_time->tm_min, estimated_time->tm_sec);
            free(estimated_time);
        }
        
        for (int i = 0; i < next_stops_count; i++) {
            free(next_stops[i]);
        }
    
    }
   
//}

}
