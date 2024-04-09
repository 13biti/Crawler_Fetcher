cmake_minimum_required(VERSION 3.21)
project(mainProgram)

set(CMAKE_CXX_STANDARD 14)
set(PROJECT_HEADERS
        Politeness.h
        UrlManager.h
        FileOperations.h)
set(PROJECT_SOURCES
        main.cpp
        Politeness.cpp
        UrlManagement.cpp
        FileOperations.cpp)
add_executable(mainProgram ${PROJECT_SOURCES} ${PROJECT_HEADERS})
target_sources(mainProgram PRIVATE test.txt)
