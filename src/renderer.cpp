#include "renderer.h"
#include "GLFW/glfw3.h"

void renderInit(GLFWwindow* window) {
    
}

void render(GLFWwindow* window) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}