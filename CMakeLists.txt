cmake_minimum_required(VERSION 3.31)
project(cp_ls_hybrid)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(schedule-generator main.cpp
        structures.cpp
        schedule.cpp
        schedule.h
        structures.h
        constraint_checker.cpp
        constraint_checker.h
        adaptive_neighborhood_selector.h
        adaptive_neighborhood_selector.cpp
        cpls_scheduler.h
        cpls_scheduler.cpp
        scheduling_data_serialization.h)

add_executable(input-data-generator test_data_generator.cpp
        structures.cpp
        schedule.cpp
        schedule.h
        structures.h
        constraint_checker.cpp
        constraint_checker.h
        adaptive_neighborhood_selector.h
        adaptive_neighborhood_selector.cpp
        scheduling_data_generator.h
        scheduling_data_generator.cpp
        cpls_scheduler.h
        cpls_scheduler.cpp
        scheduling_data_serialization.h)
