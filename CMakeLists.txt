cmake_minimum_required(VERSION 3.31..)
set(CMAKE_CXX_STANDARD 17)
project(openblocks VERSION 0.1.0)
set(OpenGL_GL_PREFERENCE "GLVND")

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(core)
add_subdirectory(client)
add_subdirectory(editor)