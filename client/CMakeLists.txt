
add_executable(client "src/main.cpp" $<TARGET_OBJECTS:openblocks>)
include_directories("../src")
target_link_libraries(client ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} OpenGL::GL OpenGL::GLU glfw glm::glm assimp ReactPhysics3D::ReactPhysics3D)