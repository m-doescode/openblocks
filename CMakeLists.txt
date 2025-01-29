cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 17)
project(openblocks VERSION 0.1.0)
set(OpenGL_GL_PREFERENCE "GLVND")

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib )

SET(BASEPATH "${CMAKE_SOURCE_DIR}")
include_directories("${BASEPATH}")
include_directories("src")

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})

find_package(glfw3 REQUIRED)

find_package(OpenGL)

find_package(glm CONFIG REQUIRED)

find_package(assimp REQUIRED)

find_package(ReactPhysics3D REQUIRED)

file(MAKE_DIRECTORY bin)

include_directories("include")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_library(openblocks ${SOURCES})
set_target_properties(openblocks PROPERTIES OUTPUT_NAME "openblocks")
target_link_libraries(openblocks ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} OpenGL::GL OpenGL::GLU glfw glm::glm assimp ReactPhysics3D::ReactPhysics3D)

# add_executable(client "client/src/main.cpp" $<TARGET_OBJECTS:openblocks>)
# include_directories("src")
# target_link_libraries(client ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} OpenGL::GL OpenGL::GLU glfw glm::glm assimp ReactPhysics3D::ReactPhysics3D)

add_subdirectory("client")
add_subdirectory("editor")