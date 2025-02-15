cmake_minimum_required(VERSION 3.21)
project(mainProgram)

set(CMAKE_CXX_STANDARD 14)
set(PROJECT_HEADERS
        QueueManager.h)
set(PROJECT_SOURCES
        cURL-Downloader.cpp
        QueueManager.cpp)
add_executable(mainProgram ${PROJECT_SOURCES} ${PROJECT_HEADERS})
target_sources(mainProgram PRIVATE test.txt)
