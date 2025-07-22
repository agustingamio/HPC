//
// Created by agamio on 7/21/2025.
//

#include "mpi_types.h"

#include "frequency.h"
#include "stop_graph.h"

void create_tm_type(MPI_Datatype *mpi_tm_type) {
    const int n_items = 9;
    const int block_lengths[9] = {1,1,1,1,1,1,1,1,1};
    MPI_Aint offsets[9];
    const MPI_Datatype types[9] = {
        MPI_INT, MPI_INT, MPI_INT,
        MPI_INT, MPI_INT, MPI_INT,
        MPI_INT, MPI_INT, MPI_INT
    };

    offsets[0] = offsetof(struct tm, tm_sec);
    offsets[1] = offsetof(struct tm, tm_min);
    offsets[2] = offsetof(struct tm, tm_hour);
    offsets[3] = offsetof(struct tm, tm_mday);
    offsets[4] = offsetof(struct tm, tm_mon);
    offsets[5] = offsetof(struct tm, tm_year);
    offsets[6] = offsetof(struct tm, tm_wday);
    offsets[7] = offsetof(struct tm, tm_yday);
    offsets[8] = offsetof(struct tm, tm_isdst);

    MPI_Type_create_struct(n_items, block_lengths, offsets, types, mpi_tm_type);
    MPI_Type_commit(mpi_tm_type);
}

void create_frequency_type(MPI_Datatype *mpi_frequency_type, const MPI_Datatype mpi_tm_type) {
    const int n_items = 4;
    const int block_lengths[4] = {1, 1, 1, 1};
    const MPI_Datatype types[4] = {
        MPI_INT,     // variant_id
        MPI_INT,     // day_type (enum)
        mpi_tm_type, // departure_time
        MPI_LONG     // frequency_average
    };

    MPI_Aint offsets[4];
    offsets[0] = offsetof(Frequency, variant_id);
    offsets[1] = offsetof(Frequency, day_type);
    offsets[2] = offsetof(Frequency, departure_time);
    offsets[3] = offsetof(Frequency, frequency_average);

    MPI_Type_create_struct(n_items, block_lengths, offsets, types, mpi_frequency_type);
    MPI_Type_commit(mpi_frequency_type);
}

void create_stop_graph_type(MPI_Datatype *mpi_stop_graph_type, const MPI_Datatype mpi_tm_type) {
    const int n_items = 7;
    const int block_lengths[7] = {
        1, 1, 1, 1, 1,
        MAX_TICKETS_PER_STOP,
        1
    };

    const MPI_Datatype types[7] = {
        MPI_INT, MPI_INT, MPI_INT, MPI_INT,
        MPI_LONG,
        mpi_tm_type,
        MPI_INT
    };

    MPI_Aint offsets[7];
    offsets[0] = offsetof(StopGraph, stop_id);
    offsets[1] = offsetof(StopGraph, variant_id);
    offsets[2] = offsetof(StopGraph, relative_stop_id);
    offsets[3] = offsetof(StopGraph, day_type);
    offsets[4] = offsetof(StopGraph, time_from_last_stop);
    offsets[5] = offsetof(StopGraph, last_tickets);
    offsets[6] = offsetof(StopGraph, ticket_count);

    MPI_Type_create_struct(n_items, block_lengths, offsets, types, mpi_stop_graph_type);
    MPI_Type_commit(mpi_stop_graph_type);
}

void create_ticket_type(MPI_Datatype *mpi_ticket_type, const MPI_Datatype mpi_tm_type) {
    const int n_items = 3;
    const int block_lengths[3] = {1, 1, 1};
    const MPI_Datatype types[3] = {mpi_tm_type, MPI_INT, MPI_INT};
    MPI_Aint offsets[3];

    offsets[0] = offsetof(Ticket, sold_date);
    offsets[1] = offsetof(Ticket, stop_id);
    offsets[2] = offsetof(Ticket, variant_id);

    MPI_Type_create_struct(n_items, block_lengths, offsets, types, mpi_ticket_type);
    MPI_Type_commit(mpi_ticket_type);
}