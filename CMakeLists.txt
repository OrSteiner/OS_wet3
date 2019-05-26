cmake_minimum_required(VERSION 3.10)
project(OS_wet3)

set(CMAKE_CXX_STANDARD 11)

include_directories(Part1)

add_executable(OS_wet3
        Part2/main.cpp
        Part1/Headers.hpp
        Part1/PCQueue.cpp
        Part1/PCQueue.hpp
        Part1/Semaphore.hpp
        Part1/Semaphore.cpp
        Part2/Game.cpp
        Part2/Game.hpp
        Part2/utils.cpp
        Part2/utils.hpp
        Part2/Thread.hpp
        Part2/Thread_Consumer.hpp)
