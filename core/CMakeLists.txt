find_package(OpenGL REQUIRED COMPONENTS OpenGL)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(OpenGL)
find_package(glm CONFIG REQUIRED)
find_package(ReactPhysics3D REQUIRED)
find_package(pugixml REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_library(openblocks ${SOURCES})
set_target_properties(openblocks PROPERTIES OUTPUT_NAME "openblocks")
target_link_libraries(openblocks ${GLEW_LIBRARIES} OpenGL::GL ReactPhysics3D::ReactPhysics3D pugixml::pugixml)
target_include_directories(openblocks PUBLIC "src" "../include")