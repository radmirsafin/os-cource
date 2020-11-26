cmake_minimum_required(VERSION 3.17)
project(memory_allocator C)

set(CMAKE_C_STANDARD 99)

add_executable(memory_allocator slab-allocator.c)
target_link_libraries(memory_allocator m)
