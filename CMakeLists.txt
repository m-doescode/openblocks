cmake_minimum_required(VERSION 3.30.0)
set(CMAKE_CXX_STANDARD 20)
project(openblocks VERSION 0.1.0)
set(OpenGL_GL_PREFERENCE "GLVND")

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -Wno-unused-parameter)
endif()

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )
set(USE_CCACHE ON)

add_subdirectory(autogen)

enable_testing()
add_subdirectory(tests)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(core)
add_subdirectory(client)
add_subdirectory(editor)


install(FILES $<TARGET_RUNTIME_DLLS:editor> TYPE BIN)