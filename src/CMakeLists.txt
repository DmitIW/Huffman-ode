add_library(poco_simple_handler SimplePocoHandler.h SimplePocoHandler.cpp)
target_link_libraries(poco_simple_handler PocoNet PocoFoundation)

set(PROGS )

foreach(item ${PROGS})
    add_executable(${item} "${item}.cpp")
    target_link_libraries(${item} amqp-cpp poco_simple_handler)
endforeach(item)