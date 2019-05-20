cmake_minimum_required(VERSION 3.10)
project(OS_wet3)

set(CMAKE_CXX_STANDARD 11)

include_directories(Part1)

add_executable(OS_wet3
        Part1/Headers.hpp
        Part1/PCQueue.cpp
        Part1/PCQueue.hpp
        Part1/Semaphore.hpp)
