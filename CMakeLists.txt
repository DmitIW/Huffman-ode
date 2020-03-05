set(PROJECT HuffmanCode)
project(${PROJECT} C CXX)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-Wall -Wextra -pedantic)

add_subdirectory(3rdParty/AMQP-CPP)
add_subdirectory(src)