cmake_minimum_required(VERSION 3.10)
project(OS_wet3)

set(CMAKE_CXX_STANDARD 11)

include_directories(Part1)

add_executable(OS_wet3
        main.cpp
        Headers.hpp
        PCQueue.cpp
        PCQueue.hpp
        Semaphore.hpp
        Semaphore.cpp)
