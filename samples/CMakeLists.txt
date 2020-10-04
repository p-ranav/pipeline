
add_executable(main main.cpp)
target_link_libraries(main PRIVATE pipeline::pipeline)

add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world PRIVATE pipeline::pipeline)
