//
// Created by agamio on 7/21/2025.
//

#include "mpi_utils.h"

#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "frequency.h"

array_node* create_nodes_array(const int nodes_count) {;
    array_node* array = malloc(sizeof(array_node) * nodes_count);

    for (int i = 0; i < nodes_count; i++) {
        array[i].variants_count = 0;
        array[i].variants = NULL;
    }

    return array;
}

void add_variant(variant_list* list, const int variant_id) {
    variant_list new_node = malloc(sizeof(struct variant_list_node));
    new_node->variant_id = variant_id;
    new_node->processed_tickets_count = 0;
    new_node->next = *list;
    *list = new_node;
}

void add_variant_to_array(array_node* slaves_array, const int variant_id, const int rank) {
    slaves_array[rank].variants_count++;
    add_variant(&slaves_array[rank].variants, variant_id);
}

int find_in_variant_list(variant_list list, const int variant_id) {
    while (list != NULL) {
        if (list->variant_id == variant_id) {
            list->processed_tickets_count++;
            if (list->processed_tickets_count % 10 == 0) {
                return 2;
            }

            return 1;
        }

        list = list->next;
    }

    return 0;
}

int find_node(const array_node* nodes_array, const int variant_id, const int nodes_count, int* update) {
    for (int index = 0; index < nodes_count; index++) {
        *update = find_in_variant_list(nodes_array[index].variants, variant_id);
        if (*update) {
            return index;
        }
    }

    return -1;
}

int find_less_overloaded_node(const array_node* nodes_array, const int nodes_count) {
    int min_variants = nodes_array[0].variants_count;
    int min_index = 0;

    for (int i = 1; i < nodes_count; i++) {
        if (nodes_array[i].variants_count < min_variants) {
            min_variants = nodes_array[i].variants_count;
            min_index = i;
        }
    }

    return min_index;
}

void mpi_print(char* message) {
    int rank;
    const time_t now = time(NULL);
    const struct tm* local_time = localtime(&now);
    char time_str[20];

    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("DATETIME: [%s] | RANK: %d | MESSAGE: %s\n", time_str, rank, message);
}

void free_list(variant_list list) {
    variant_list current = list;
    variant_list next_node;

    while (current != NULL) {
        next_node = current->next;
        free(current);
        current = next_node;
    }
}

void free_slaves_array(array_node* slaves_array, const int slaves_count) {
    for (int i = 0; i < slaves_count; i++) {
        free_list(slaves_array[i].variants);
    }

    free(slaves_array);
}




void send_graph_updates(StopGraph* graph, const int stops_count,
                        const Ticket ticket, const MPITypes types)
{
    int comm_size, updates_receiver;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    MPI_Recv(&updates_receiver, 1, MPI_INT, 0, send_updates, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int variant_stops_count = 0;
    for (int i = 0; i < stops_count; i++) {
        if (graph[i].variant_id == ticket.variant_id) {
            variant_stops_count++;
        }
    }

    int index = 0;
    StopGraph* stops_list[variant_stops_count];
    for (int i = 0; i < stops_count; i++) {
        if (graph[i].variant_id == ticket.variant_id) {
            stops_list[index++] = &graph[i];
        }
    }

    MPI_Send(&variant_stops_count, 1, MPI_INT, updates_receiver, send_updates, MPI_COMM_WORLD);
    MPI_Send(*stops_list, variant_stops_count, types.stop_graph_type, updates_receiver, send_updates, MPI_COMM_WORLD);
}

void recv_graph_updates(StopGraph* graph, const int stops_count, const MPITypes types)
{
    int update_size, update_sender;
    MPI_Recv(&update_sender, 1, MPI_INT, 0, recv_updates, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&update_size, 1, MPI_INT, update_sender, send_updates, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    StopGraph* updates = malloc(update_size * sizeof(StopGraph));
    MPI_Recv(updates, update_size, types.stop_graph_type, update_sender, send_updates, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for (int j = 0; j < update_size; j++) {
        for (int i = 0; i < stops_count; i++) {
            if (graph[i].stop_id == updates[j].stop_id &&
                graph[i].variant_id == updates[j].variant_id &&
                graph[i].day_type == updates[j].day_type) {
                graph[i].time_from_last_stop = updates[j].time_from_last_stop;
                graph[i].next_arrival_time = updates[j].next_arrival_time;
                break;
                }
        }
    }

    free(updates);
}

void receive_structures(Frequency** frequencies, int* freq_count,
                                  StopGraph** graph, int* stops_count,
                                  const MPITypes types)
{
    MPI_Bcast(freq_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    *frequencies = (Frequency*)malloc(*freq_count * sizeof(Frequency));
    MPI_Bcast(*frequencies, *freq_count, types.frequency_type, 0, MPI_COMM_WORLD);

    MPI_Bcast(stops_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    *graph = (StopGraph*)malloc(*stops_count * sizeof(StopGraph));
    MPI_Bcast(*graph, *stops_count, types.stop_graph_type, 0, MPI_COMM_WORLD);
}