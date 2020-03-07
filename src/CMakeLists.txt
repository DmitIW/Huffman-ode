add_library(QueueHandler QueueConnectionHandler.h QueueConnectionHandler.cpp utility.cpp)
target_link_libraries(QueueHandler rabbitmq)

add_library(Worker Worker.h Worker.cpp)

SET (WORKERS listner
             speaker)

FOREACH(item ${WORKERS})
    add_executable(${item} "${item}.cpp")
    target_link_libraries(${item} Wroker QueueHandler rabbitmq)
ENDFOREACH()