include_directories(utility)
add_subdirectory(utility)

include_directories(connectionHandler)
add_subdirectory(connectionHandler)

include_directories(workers)
add_subdirectory(workers)

include_directories(DirectoryReader)
add_subdirectory(DirectoryReader)

SET (PROGS listner
             speaker)

FOREACH(item ${PROGS})
    add_executable(${item} "${item}.cpp")
    target_link_libraries(${item} utility_h Workers QueueConnectionHandler DirectoryReader rabbitmq)
ENDFOREACH()