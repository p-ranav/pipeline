add_executable(for_each for_each.cpp)
target_link_libraries(for_each PRIVATE pipeline::pipeline)

add_executable(for_each_return for_each_return.cpp)
target_link_libraries(for_each_return PRIVATE pipeline::pipeline)

add_executable(fork_into fork_into.cpp)
target_link_libraries(fork_into PRIVATE pipeline::pipeline)

add_executable(pipe pipe.cpp)
target_link_libraries(pipe PRIVATE pipeline::pipeline)

add_executable(pipe_reference pipe_reference.cpp)
target_link_libraries(pipe_reference PRIVATE pipeline::pipeline)

add_executable(unzip_into unzip_into.cpp)
target_link_libraries(unzip_into PRIVATE pipeline::pipeline)

add_executable(unzip_into_return unzip_into_return.cpp)
target_link_libraries(unzip_into_return PRIVATE pipeline::pipeline)

add_executable(unzip_into_single_functor unzip_into_single_functor.cpp)
target_link_libraries(unzip_into_single_functor PRIVATE pipeline::pipeline)
