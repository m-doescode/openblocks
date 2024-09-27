#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "renderer.h"

void errorCatcher(int id, const char* str);

int main() {
    glfwSetErrorCallback(errorCatcher);

    glfwInit();
    GLFWwindow *window = glfwCreateWindow(1200, 900, "GLTest", NULL, NULL);

    glfwMakeContextCurrent(window);
    glewInit();

    renderInit(window);

    do {
        render(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(!glfwWindowShouldClose(window));
}

void errorCatcher(int id, const char* str) {
    printf("Something *terrible* happened. Here's the briefing: [%d] %s\n", id, str);
}