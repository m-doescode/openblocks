find_package(OpenGL REQUIRED COMPONENTS OpenGL)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(OpenGL)
find_package(glm CONFIG REQUIRED)
find_package(ReactPhysics3D REQUIRED)
find_package(pugixml 1.15 REQUIRED)
find_package(Freetype)

find_package(Stb REQUIRED)
include_directories(${Stb_INCLUDE_DIR})

# PkgConfig packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LUAJIT REQUIRED luajit)
link_directories(${LUAJIT_LIBRARY_DIRS})

### Autogen
file(GLOB_RECURSE AUTOGEN_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src" "src/objects/*.h" "src/datatypes/*.h" "src/enum/*.h")

# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Custom%20Commands.html
foreach (SRC ${AUTOGEN_SOURCES})
    string(REGEX REPLACE "[.]h$" ".cpp" OUT_SRC_NAME ${SRC})
    set(SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/${SRC}")
    set(OUT_PATH "${CMAKE_BINARY_DIR}/generated/${OUT_SRC_NAME}")

    add_custom_command(
        OUTPUT "${OUT_PATH}"
        DEPENDS autogen
        DEPENDS "${SRC_PATH}"
        COMMAND "$<TARGET_FILE:autogen>" "${CMAKE_CURRENT_SOURCE_DIR}/src" "${SRC_PATH}" "${OUT_PATH}"
    )

    list(APPEND AUTOGEN_OUTS "${OUT_PATH}")
endforeach()

### /Autogen

# Add version info into the build
include(gitversion)

add_custom_target(autogen_build ALL
    DEPENDS ${AUTOGEN_OUTS}
)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
list(APPEND SOURCES ${AUTOGEN_OUTS})
list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/src/version.cpp)
add_library(openblocks STATIC ${SOURCES})
set_target_properties(openblocks PROPERTIES OUTPUT_NAME "openblocks")
target_link_directories(openblocks PUBLIC ${LUAJIT_LIBRARY_DIRS})
target_link_libraries(openblocks ${GLEW_LIBRARIES} ${LUAJIT_LIBRARIES} OpenGL::GL ReactPhysics3D::ReactPhysics3D pugixml::pugixml Freetype::Freetype)
target_include_directories(openblocks PUBLIC "src" "../include" ${LUAJIT_INCLUDE_DIRS})
add_dependencies(openblocks autogen_build autogen)

# Windows-specific dependencies
if(WIN32)
    target_link_libraries(openblocks shell32.lib)
endif()