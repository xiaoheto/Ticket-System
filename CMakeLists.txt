cmake_minimum_required(VERSION 3.22)
project(Ticket_System_Copy)

set(CMAKE_CXX_STANDARD 14)

include_directories(BPT)
include_directories(STLite)

add_executable(code
        main.cpp
    )
