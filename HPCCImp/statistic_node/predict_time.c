
#include <../ticket.h>
#include <../day_type.h>
#include <../stop_graph.h>
#include <../utils.h>

int get_ticket(Ticket* ticket){
    //TODO: Ver como vamos a resolver la obtencion del ticket, de momento, datos de prueba:
    ticket->sold_date.tm_year = 125;
    ticket->sold_date.tm_mon = 9; // Octubre
    ticket->sold_date.tm_mday = 15; // 15 de Octubre
    ticket->sold_date.tm_hour = 10; // 10 AM
    ticket->sold_date.tm_min = 30; // 30 minutos
    ticket->sold_date.tm_sec = 0; // 0 segundos
    ticket->sold_date.tm_isdst = -1; // No se sabe si es horario de verano
    ticket->stop_id = 123; // ID de la parada
    ticket->variant_id = 456; // ID de la variante
    return 1; // Retorna 1 si se obtuvo un ticket, 0 si no
}

int get_slave_by_variant(int variant_id) {
    //TODO: Implementar la logica para obtener el id del nodo esclavo que tiene la variante
    return 0;
}

StopGraph** get_stop_graph_from_slave(int slave_id, int* stop_graph_count) {
    //TODO: Implementar la logica para obtener el grafo de paradas del nodo esclavo
    *stop_graph_count = 0; // Asignar el conteo de paradas
    return NULL; // Retornar el grafo de paradas
}

void calculate_estimated_time(StopGraph stop_graph, struct tm sold_date, struct tm* estimated_time) {
    //TODO: Calcula el tiempo estimado para la parada
    long time_from_last_stop = stop_graph.time_from_last_stop;
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
        StopGraph* graph = get_stop_graph_from_slave(slave_id, &stop_graph_count);
        int relative_stop = get_relative_stop_by_stop_id(graph, stop_graph_count, stop_id, variant_id);
        //se queda con todas las paradas que tengan id relativo de parada superior al leifo en el ticket
        int next_stops_count = 0;
        for (int i = 0; i<stop_graph_count; i++){
            if (graph[i].relative_stop_id > relative_stop && graph[i].day_type == day_type && graph[i].variant_id == variant_id) {
                next_stops_count ++;
            }
        }

        StopGraph* next_stops = malloc(next_stops_count * sizeof(StopGraph*));
        if (!next_stops) {
            fprintf(stderr, "Error allocating memory for next stops\n");
            return -1;
        }

        int index = 0;
        for (int i = 0; i<stop_graph_count; i++){
            if (graph[i].relative_stop_id > relative_stop && graph[i].day_type == day_type && graph[i].variant_id == variant_id) {
                next_stops[index++] = graph[i];
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
            printf("Estimated time for stop %d: %02d:%02d:%02d\n", next_stops[i].relative_stop_id, 
                estimated_time->tm_hour, estimated_time->tm_min, estimated_time->tm_sec);
            free(estimated_time);
        }
        
        for (int i = 0; i < next_stops_count; i++) {
            free(next_stops[i]);
        }
    
    }
   
//}

}
