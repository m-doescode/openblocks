find_package(OpenGL REQUIRED COMPONENTS OpenGL)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(OpenGL)
find_package(glm CONFIG REQUIRED)
find_package(ReactPhysics3D REQUIRED)
find_package(pugixml 1.15 REQUIRED)

find_package(Stb REQUIRED)
include_directories(${Stb_INCLUDE_DIR})

# PkgConfig packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LUAJIT REQUIRED luajit)

# Run autogen
add_custom_target(build_autogen ALL
    COMMAND "${CMAKE_BINARY_DIR}/autogen/autogen" "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/src/objects" "${CMAKE_BINARY_DIR}/generated"
)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h" "${CMAKE_BINARY_DIR}/generated/*.cpp")
add_library(openblocks STATIC ${SOURCES})
set_target_properties(openblocks PROPERTIES OUTPUT_NAME "openblocks")
target_link_libraries(openblocks ${GLEW_LIBRARIES} ${LUAJIT_LIBRARIES} OpenGL::GL ReactPhysics3D::ReactPhysics3D pugixml::pugixml)
target_include_directories(openblocks PUBLIC "src" "../include" ${LUAJIT_INCLUDE_DIR})

# Windows-specific dependencies
if(WIN32)
    target_link_libraries(openblocks shell32.lib)
endif()