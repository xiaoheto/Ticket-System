cmake_minimum_required(VERSION 3.22)
project(Ticket_System_Copy)

set(CMAKE_CXX_STANDARD 14)

include_directories(BPT)
include_directories(STLite)
include_directories(Tool)

add_executable(code
        main.cpp
        BPT/BPlusTree.hpp
        src/User.h
        BPT/CommandParser.cpp
        BPT/CommandParser.h
        BPT/MyChar.h
        BPT/MemoryRiver.h
        src/MyError.h
        src/UserManagement.cpp
        src/UserManagement.h
        STLite/map.hpp
        STLite/utility.h
        src/TrainManagement.cpp
        src/TrainManagement.h
        src/Train.h
        src/Time.cpp
        src/Time.h
)
