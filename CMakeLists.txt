cmake_minimum_required(VERSION 3.21)
project(mainProgram)

set(CMAKE_CXX_STANDARD 14)
set(PROJECT_HEADERS
        Politeness.h
        UrlManager.h
        FileOperations.h
        ./messageManagerClass/messageManagerClass.h)
set(PROJECT_SOURCES
        main.cpp
        Politeness.cpp
        UrlManagement.cpp
        FileOperations.cpp
        ./messageManagerClass/messageManagerClass.cpp)
add_executable(mainProgram ${PROJECT_SOURCES} ${PROJECT_HEADERS})
target_sources(mainProgram PRIVATE test.txt)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} SimpleAmqpClient rabbitmq ${CMAKE_THREAD_LIBS_INIT} )

