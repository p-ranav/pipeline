
add_executable(main main.cpp)
target_link_libraries(main PRIVATE pipeline::pipeline)

add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world PRIVATE pipeline::pipeline)

add_executable(sum_diff sum_diff.cpp)
target_link_libraries(sum_diff PRIVATE pipeline::pipeline)

add_executable(sum_diff_async sum_diff_async.cpp)
target_link_libraries(sum_diff_async PRIVATE pipeline::pipeline)
