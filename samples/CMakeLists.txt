
add_executable(main main.cpp)
target_link_libraries(main PRIVATE pipeline::pipeline)

add_executable(for_each for_each.cpp)
target_link_libraries(for_each PRIVATE pipeline::pipeline)

add_executable(for_each_async for_each_async.cpp)
target_link_libraries(for_each_async PRIVATE pipeline::pipeline)

add_executable(fork_into fork_into.cpp)
target_link_libraries(fork_into PRIVATE pipeline::pipeline)
