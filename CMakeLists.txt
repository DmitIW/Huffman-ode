set(PROJECT HuffmanCode)
project(${PROJECT} C CXX)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
add_definitions(-Wall -Wextra -pedantic -O2)

IF (NOT EXISTS rabbitmq-c/.git)
    message("Rabbitmq-c doesn't exists. Loading submodule...")
    execute_process(
            COMMAND sh -c "git submodule update --init"
            WORKING_DIRECTORY ./
    )
ENDIF()

include_directories(rabbitmq-c/librabbitmq)
include_directories(rabbitmq-c)

add_subdirectory(rabbitmq-c)
add_subdirectory(src)