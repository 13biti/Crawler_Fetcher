cmake_minimum_required(VERSION 3.21)
project(MessageReceiver_cpp)

set(CMAKE_CXX_STANDARD 14)

add_executable(MessageReceiver_cpp main.cpp )
target_link_libraries(MessageReceiver_cpp -lSimpleAmqpClient -lrabbitmq)

