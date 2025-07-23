//
// Created by agamio on 7/21/2025.
//

#include "mpi_types.h"

#include "frequency.h"
#include "stop_graph.h"

void create_frequency_type(MPI_Datatype *mpi_frequency_type) {
    const int n_items = 4;
    int block_lengths[4] = {1, 1, (int)sizeof(struct tm), 1};
    MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_BYTE, MPI_LONG};

    Frequency temp;
    MPI_Aint offsets[4], base;
    MPI_Get_address(&temp, &base);
    MPI_Get_address(&temp.variant_id, &offsets[0]);
    MPI_Get_address(&temp.day_type, &offsets[1]);
    MPI_Get_address(&temp.departure_time, &offsets[2]);
    MPI_Get_address(&temp.frequency_average, &offsets[3]);

    for (int i = 0; i < n_items; i++)
        offsets[i] -= base;

    MPI_Type_create_struct(n_items, block_lengths, offsets, types, mpi_frequency_type);
    MPI_Type_commit(mpi_frequency_type);
}

void create_stop_graph_type(MPI_Datatype *mpi_stop_graph_type) {
    const int n_items = 7;
    int block_lengths[7] = {
        1, 1, 1, 1, 1,
        MAX_TICKETS_PER_STOP * (int)sizeof(struct tm),
        1
    };

    MPI_Datatype types[7] = {
        MPI_INT, MPI_INT, MPI_INT, MPI_INT,
        MPI_LONG,
        MPI_BYTE,
        MPI_INT
    };

    StopGraph temp;
    MPI_Aint offsets[7], base;
    MPI_Get_address(&temp, &base);
    MPI_Get_address(&temp.stop_id, &offsets[0]);
    MPI_Get_address(&temp.variant_id, &offsets[1]);
    MPI_Get_address(&temp.relative_stop_id, &offsets[2]);
    MPI_Get_address(&temp.day_type, &offsets[3]);
    MPI_Get_address(&temp.time_from_last_stop, &offsets[4]);
    MPI_Get_address(&temp.last_tickets, &offsets[5]);
    MPI_Get_address(&temp.ticket_count, &offsets[6]);

    for (int i = 0; i < n_items; i++)
        offsets[i] -= base;

    MPI_Type_create_struct(n_items, block_lengths, offsets, types, mpi_stop_graph_type);
    MPI_Type_commit(mpi_stop_graph_type);
}

void create_ticket_type(MPI_Datatype *mpi_ticket_type) {
    const int n_items = 3;
    int block_lengths[3] = {(int)sizeof(struct tm), 1, 1};
    MPI_Datatype types[3] = {MPI_BYTE, MPI_INT, MPI_INT};

    Ticket temp;
    MPI_Aint offsets[3], base;
    MPI_Get_address(&temp, &base);
    MPI_Get_address(&temp.sold_date, &offsets[0]);
    MPI_Get_address(&temp.stop_id, &offsets[1]);
    MPI_Get_address(&temp.variant_id, &offsets[2]);

    for (int i = 0; i < n_items; i++)
        offsets[i] -= base;

    MPI_Type_create_struct(n_items, block_lengths, offsets, types, mpi_ticket_type);
    MPI_Type_commit(mpi_ticket_type);
}