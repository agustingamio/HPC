cmake_minimum_required(VERSION 3.28.3)
project(HPCCConcept C)

set(CMAKE_BUILD_TYPE Debug)

add_executable(HPCCConcept main.c planned_route.c frequency.c day_type.c ticket.c
        stop_graph.c
        stop_graph.h
        ticket_processor.c
        ticket_processor.h
        utils.c
        utils.h)
