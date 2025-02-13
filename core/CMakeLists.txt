find_package(OpenGL REQUIRED COMPONENTS OpenGL)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})

find_package(OpenGL)
find_package(glm CONFIG REQUIRED)
# find_package(assimp REQUIRED)
find_package(ReactPhysics3D REQUIRED)
find_package(pugixml REQUIRED)

file(MAKE_DIRECTORY bin)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_library(openblocks ${SOURCES})
set_target_properties(openblocks PROPERTIES OUTPUT_NAME "openblocks")
target_link_libraries(openblocks ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} OpenGL::GL OpenGL::GLU ReactPhysics3D::ReactPhysics3D pugixml::pugixml)
target_include_directories(openblocks PUBLIC "src" "../include")