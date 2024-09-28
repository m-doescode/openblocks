cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 17)
project(GLTest VERSION 0.1.0)
set(OpenGL_GL_PREFERENCE "GLVND")

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

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

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "gltest")
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} OpenGL::GL OpenGL::GLU glfw glm::glm assimp ReactPhysics3D::ReactPhysics3D)