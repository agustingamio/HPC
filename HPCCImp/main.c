#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "frequency.h"
#include "mpi_types.h"
#include "stop_graph.h"
#include "ticket.h"
#include "master_node/master.h"
#include "slave_node/slave.h"

int master() {
    // INIT MPI and send the structures
    int rank, comm_size;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    MPITypes types;
    create_tm_type(&types.tm_type);
    create_ticket_type(&types.ticket_type, types.tm_type);
    create_stop_graph_type(&types.stop_graph_type, types.tm_type);
    create_frequency_type(&types.frequency_type, types.tm_type);


    if (rank == 0) {
        // MASTER LOGIC
        main_master(types);
    } else if (rank < comm_size) {
        // SLAVE LOGIC
        main_slave(types);
    } else {
        // STATISTIC LOGIC
    }

    // Clean up types
    MPI_Type_free(&types.ticket_type);
    MPI_Type_free(&types.stop_graph_type);
    MPI_Type_free(&types.frequency_type);
    MPI_Type_free(&types.tm_type);

    MPI_Finalize();
    return 0;
}
