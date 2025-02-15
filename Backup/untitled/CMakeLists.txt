cmake_minimum_required(VERSION 3.21)
project(untitled)

set(CMAKE_CXX_STANDARD 14)
set(PROJECT_HEADERS
        )
set(PROJECT_SOURCES
        main.cpp
        )
add_executable(untitled main.cpp QueueManager.cpp)
target_link_libraries(untitled -lcurl -lrabbitmq)
