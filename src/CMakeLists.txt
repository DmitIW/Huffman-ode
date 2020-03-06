add_executable(${PROJECT} QueueConnectionHandler.h QueueConnectionHandler.cpp utility.cpp listner.cpp)
target_link_libraries(${PROJECT} rabbitmq)