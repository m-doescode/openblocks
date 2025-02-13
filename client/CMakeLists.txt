find_package(glfw3 REQUIRED)

add_executable(client "src/main.cpp")
target_link_libraries(client PRIVATE openblocks glfw)