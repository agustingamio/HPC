cmake_minimum_required(VERSION 3.28.3)
project(HPCCConcept C)

set(CMAKE_BUILD_TYPE Debug)
find_package(MPI REQUIRED)
include_directories(SYSTEM "C:/Program Files (x86)/Microsoft SDKs/MPI/Include")
link_directories("C:/Program Files (x86)/Microsoft SDKs/MPI/Lib/x64")

add_executable(HPCCConcept main.c planned_route.c frequency.c day_type.c ticket.c
        stop_graph.c
        stop_graph.h
        ticket_processor.c
        ticket_processor.h
        utils.c
        utils.h
        predict_time.c
        predict_time.h
        master_node/master.c
        mpi_utils.c
        mpi_utils.h
        master_node/master.h
        mpi_types.c
        mpi_types.h
        slave_node/slave.c
        slave_node/slave.h
        checkpoints_node/checkpoints.c
        checkpoints_node/checkpoints.h)

target_link_libraries(HPCCConcept PRIVATE MPI::MPI_C)