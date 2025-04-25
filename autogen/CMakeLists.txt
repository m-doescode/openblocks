cmake_minimum_required(VERSION 3.30.0)
find_package(Clang REQUIRED)

add_executable(autogen
    src/main.cpp
    src/util.cpp
    src/cache.cpp
)

set_target_properties(autogen PROPERTIES OUTPUT_NAME "autogen")
target_link_libraries(autogen -lclang)
target_include_directories(autogen PUBLIC "src" ${CLANG_INCLUDE_DIRS})