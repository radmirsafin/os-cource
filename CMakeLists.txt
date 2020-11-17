cmake_minimum_required(VERSION 3.17)
project(memory_allocator)

set(CMAKE_CXX_STANDARD 98)

add_executable(test_simple_allocator test_simple_allocator.cpp simple_allocator.cpp)
