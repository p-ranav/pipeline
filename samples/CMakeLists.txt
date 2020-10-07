add_executable(for_each for_each.cpp)
target_link_libraries(for_each PRIVATE pipeline::pipeline)

add_executable(fork_into fork_into.cpp)
target_link_libraries(fork_into PRIVATE pipeline::pipeline)

add_executable(pipe_reference pipe_reference.cpp)
target_link_libraries(pipe_reference PRIVATE pipeline::pipeline)

add_executable(unzip_into unzip_into.cpp)
target_link_libraries(unzip_into PRIVATE pipeline::pipeline)

add_executable(unzip_into_single unzip_into_single.cpp)
target_link_libraries(unzip_into_single PRIVATE pipeline::pipeline)
