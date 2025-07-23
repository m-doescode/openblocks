include(${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake)

add_executable(client "src/main.cpp")
target_link_libraries(client PRIVATE openblocks glfw)
add_dependencies(client openblocks)