#include <mpi.h>
#include <stdlib.h>

#include "../mpi_utils.h"
#include "../mpi_types.h"
#include "../ticket.h"

int get_slave_for_variantId(array_node* slaves_array, const int variant_id, const int slaves_count) {
    const int slave = find_slave(slaves_array, variant_id, slaves_count);

    if (slave == -1) {
        const int less_overloaded_slave = find_less_overloaded_slave(slaves_array, slaves_count);
        add_variant_to_array(slaves_array, variant_id, slaves_count);
        return less_overloaded_slave;
    }

    return slave;
}

void main_master(const MPITypes types) {
    // Initialise values
    const int tickets_count = 0, comm_size = 0;
    const Ticket* tickets = NULL;
    MPI_Comm_size(MPI_COMM_WORLD, comm_size);
    read_tickets_csv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\tickets_output.csv", &tickets, tickets_count);
    array_node* slaves_array = create_slaves_array(comm_size);

    // Send them to process in a controlled way
    for (int i = 0; i < tickets_count; i++) {
        const int slave = get_slave_for_variantId(slaves_array, tickets[i].variant_id, comm_size);
        MPI_Send(&tickets[i], 1, types.ticket_type, slave, 0, MPI_COMM_WORLD);

        _sleep(1);
    }
}
