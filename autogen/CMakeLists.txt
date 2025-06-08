cmake_minimum_required(VERSION 3.30.0)
find_package(Clang REQUIRED)

add_executable(autogen
    src/main.cpp
    src/util.cpp
    src/object/analysis.cpp
    src/object/codegen.cpp
    src/data/analysis.cpp
    src/data/codegen.cpp
    src/enum/analysis.cpp
    src/enum/codegen.cpp
)

set_target_properties(autogen PROPERTIES OUTPUT_NAME "autogen")
target_link_libraries(autogen ${CLANG_LIBRARY})
target_include_directories(autogen PUBLIC "src" ${CLANG_INCLUDE_DIR})