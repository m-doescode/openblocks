#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "rendering/renderer.h"
#include "camera.h"

void errorCatcher(int id, const char* str);

Camera camera(glm::vec3(0.0, 0.0, 3.0));

// void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
// void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

int main() {
    glfwSetErrorCallback(errorCatcher);

    glfwInit();
    GLFWwindow *window = glfwCreateWindow(1200, 900, "GLTest", NULL, NULL);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    glfwMakeContextCurrent(window);
    glewInit();

    renderInit(window);

    do {
        processInput(window);
        render(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(!glfwWindowShouldClose(window));

    glfwTerminate();
    return 0;
}

void errorCatcher(int id, const char* str) {
    printf("Something *terrible* happened. Here's the briefing: [%d] %s\n", id, str);
}

float lastTime;
void processInput(GLFWwindow* window) {
    float deltaTime = glfwGetTime() - lastTime;
    lastTime = glfwGetTime();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processMovement(DIRECTION_FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processMovement(DIRECTION_BACKWARDS, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processMovement(DIRECTION_LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processMovement(DIRECTION_RIGHT, deltaTime);
}

bool mouseCapturing = false;
float lastX = 0, lastY = 0;
bool mouseFirst = true;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseCapturing) return;

    if (mouseFirst) {
        lastX = xpos, lastY = ypos;
        mouseFirst = false;
    }

    float deltaX = xpos - lastX, deltaY =  ypos - lastY;
    lastX = xpos, lastY = ypos;

    camera.processRotation(deltaX, deltaY);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_2)
        mouseCapturing = action == GLFW_PRESS;
    if (mouseCapturing) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        mouseFirst = true;
    }
}
