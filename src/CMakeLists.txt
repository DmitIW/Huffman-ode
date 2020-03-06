add_library(poco_simple_handler SimplePocoHandler.h SimplePocoHandler.cpp AsioHandler.cpp AsioHandler.h)
target_link_libraries(poco_simple_handler PocoNet PocoFoundation)

set(PROGS recieve
          send)


foreach(item ${PROGS})
    add_executable(${item} "${item}.cpp")
    target_link_libraries(${item} amqpcpp poco_simple_handler)
endforeach(item)