//
// Created by agamio on 7/21/2025.
//

#ifndef MPI_TYPES_H
#define MPI_TYPES_H
#include <mpi.h>

typedef struct {
    MPI_Datatype ticket_type;
    MPI_Datatype stop_graph_type;
    MPI_Datatype frequency_type;
} MPITypes;

void create_frequency_type(MPI_Datatype *mpi_frequency_type);

void create_stop_graph_type(MPI_Datatype *mpi_stop_graph_type);

void create_ticket_type(MPI_Datatype *mpi_ticket_type);

#endif //MPI_TYPES_H
