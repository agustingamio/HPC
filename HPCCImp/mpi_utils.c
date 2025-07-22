//
// Created by agamio on 7/21/2025.
//

#include "mpi_utils.h"

#include <stddef.h>
#include <stdlib.h>

array_node* create_slaves_array(const int slaves_count) {;
    array_node* array = malloc(sizeof(array_node) * slaves_count);

    for (int i = 0; i < slaves_count; i++) {
        array[i].variants_count = 0;
        array[i].variants = NULL;
    }

    return array;
}

void add_variant(variant_list list, const int variant_id) {
    variant_list new_node = malloc(sizeof(struct variant_list_node));
    new_node->variant_id = variant_id;
    new_node->next = list;
    list = new_node;
}

void add_variant_to_array(array_node* slaves_array, const int variant_id, const int rank) {
    slaves_array[rank].variants_count++;
    add_variant(slaves_array[rank].variants, variant_id);
}

int find_in_variant_list(variant_list list, const int variant_id) {
    while (list != NULL) {
        if (list->variant_id == variant_id) {
            return 1;
        }

        list = list->next;
    }

    return 0;
}

int find_slave(const array_node* slaves_array, const int variant_id, const int slaves_count) {
    for (int index = 0; index < slaves_count; index++) {
        if (find_in_variant_list(slaves_array[index].variants, variant_id)) {
            return index;
        }
    }

    return -1;
}

int find_less_overloaded_slave(const array_node* slaves_array, const int slaves_count) {
    int min_variants = slaves_array[0].variants_count;
    int min_index = 0;

    for (int i = 1; i < slaves_count; i++) {
        if (slaves_array[i].variants_count < min_variants) {
            min_variants = slaves_array[i].variants_count;
            min_index = i;
        }
    }

    return min_index;
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