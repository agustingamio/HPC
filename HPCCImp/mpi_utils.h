#ifndef MPI_UTILS_H
#define MPI_UTILS_H
#include "frequency.h"
#include "mpi_types.h"
#include "stop_graph.h"
#include "ticket.h"

// Create dynamic list
struct variant_list_node {
    int variant_id;
    int processed_tickets_count;
    struct variant_list_node* next;
};

typedef struct variant_list_node *variant_list;

typedef struct array_node_struct {
    int variants_count;
    variant_list variants;
} array_node;

enum tag_types {
    ticket_to_process = 0,
    send_updates = 1,
    recv_updates = 2,
    shutdown_signal = 3
};

array_node* create_slaves_array(const int slaves_count);
void add_variant_to_array(array_node* slaves_array, const int variant_id, const int rank);
int find_slave(const array_node* slaves_array, const int variant_id, const int slaves_count, int* update);
int find_less_overloaded_slave(const array_node* slaves_array, const int slaves_count);
void mpi_print(char* message);
void free_list(variant_list list);
void free_slaves_array(array_node* slaves_array, const int slaves_count);

void send_graph_updates(StopGraph* graph, const int stops_count, const Ticket ticket, const MPITypes types);
void recv_graph_updates(StopGraph* graph, const int stops_count, const Ticket ticket, const MPITypes types);
void receive_structures(Frequency** frequencies, int* freq_count, StopGraph** graph, int* stops_count, const MPITypes types);
#endif
