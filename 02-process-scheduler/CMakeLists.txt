cmake_minimum_required(VERSION 3.17)
project(round-robin-scheduler C)

set(CMAKE_C_STANDARD 99)

add_executable(round-robin-scheduler round-robin-scheduler.c)
target_link_libraries(round-robin-scheduler m)
