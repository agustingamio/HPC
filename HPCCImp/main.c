#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>

#include "mpi_types.h"
#include "master_node/master.h"
#include "slave_node/slave.h"
#include "predict_time.h"
#include "checkpoints_node/checkpoints.h"

int main(int argc, char** argv) {
    // TODO: Manage all MPI status
    int rank, comm_size;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    MPITypes types;
    create_ticket_type(&types.ticket_type);
    create_stop_graph_type(&types.stop_graph_type);
    create_frequency_type(&types.frequency_type);

    const int statistical_nodes_amount = argv[1] ? atoi(argv[1]) : 1;

    if (rank == 0) {
        main_master(types, statistical_nodes_amount);
    } else if (rank == comm_size - 1) {
        main_checkpoint(types, statistical_nodes_amount);
    } else if (rank < comm_size - statistical_nodes_amount - 1) {
        main_slave(types);
    } else {
        main_predictor(types);
    }

    MPI_Type_free(&types.ticket_type);
    MPI_Type_free(&types.stop_graph_type);
    MPI_Type_free(&types.frequency_type);

    MPI_Finalize();
    return 0;
}
