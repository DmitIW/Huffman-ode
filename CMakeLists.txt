set(PROJECT HuffmanCode)
project(${PROJECT} C CXX)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
add_definitions(-Wall -Wextra -pedantic)

set(AMQP_PATH ${CMAKE_SOURCE_DIR}/AMQP-CPP)
if(NOT EXISTS ${AMQP_PATH}/.git)
    MESSAGE("Submodule not init: AMQP-CPP")
    EXECUTE_PROCESS(
            COMMAND sh -c "git submodule update --init"
            WORKING_DIRECTORY ${AMQP_PATH}
    )
ENDIF()

set(BOOST_PATH ${CMAKE_SOURCE_DIR}/asio)
if(NOT EXISTS ${BOOST_PATH}/.git)
    MESSAGE("Submodule not init: asio")
    EXECUTE_PROCESS(
            COMMAND sh -c "git submodule update --init"
            WORKING_DIRECTORY ${BOOST_PATH}
    )
ENDIF()

include_directories(AMQP-CPP)
include_directories(asio)

add_subdirectory(AMQP-CPP)
add_subdirectory(asio)
add_subdirectory(src)