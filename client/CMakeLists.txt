find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(glfw3 REQUIRED)

add_executable(client "src/main.cpp")
target_link_libraries(client PRIVATE ${SDL2_LIBRARIES} openblocks glfw)
add_dependencies(client openblocks)