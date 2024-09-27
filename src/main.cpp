#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>
#include <stdio.h>
#include <vector>

#include "rendering/part.h"
#include "rendering/renderer.h"
#include "camera.h"

void errorCatcher(int id, const char* str);

Camera camera(glm::vec3(0.0, 0.0, 3.0));
std::vector<Part> parts;

int mode = 0;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
// void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

int main() {
    glfwSetErrorCallback(errorCatcher);

    glfwInit();
    GLFWwindow *window = glfwCreateWindow(1200, 900, "GLTest", NULL, NULL);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    glfwMakeContextCurrent(window);
    glewInit();

    parts.push_back(Part {
        .position = glm::vec3(0),
        .rotation = glm::vec3(0),
        .scale = glm::vec3(1, 1, 1),
        .material = Material {
            .diffuse = glm::vec3(1.0f, 0.5f, 0.31f),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 32.0f,
        }
    });

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

    if (mode == 2) {
        float shiftFactor = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? -0.5 : 0.5;
        shiftFactor *= deltaTime;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            parts.back().rotation.x += shiftFactor;
        }
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
            parts.back().rotation.y += shiftFactor;
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            parts.back().rotation.z += shiftFactor;
        }
    }
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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        parts.push_back(Part {
            .position = camera.cameraPos + camera.cameraFront * glm::vec3(3),
            .rotation = glm::vec3(0),
            .scale = glm::vec3(1, 1, 1),
            .material = Material {
                .diffuse = glm::vec3(1.0f, 0.5f, 0.31f),
                .specular = glm::vec3(0.5f, 0.5f, 0.5f),
                .shininess = 32.0f,
            }
        });
    }

    float shiftFactor = (mods & GLFW_MOD_SHIFT) ? -0.2 : 0.2;
    if (mode == 0) {
        if (key == GLFW_KEY_X && action == GLFW_PRESS) {
            parts.back().position.x += shiftFactor;
        }
        if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
            parts.back().position.y += shiftFactor;
        }
        if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
            parts.back().position.z += shiftFactor;
        }
    } else if (mode == 1) {
        if (key == GLFW_KEY_X && action == GLFW_PRESS) {
            parts.back().scale.x += shiftFactor;
        }
        if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
            parts.back().scale.y += shiftFactor;
        }
        if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
            parts.back().scale.z += shiftFactor;
        }
    }
    
    if (key == GLFW_KEY_M && action == GLFW_PRESS) mode = 0;
    if (key == GLFW_KEY_E && action == GLFW_PRESS) mode = 1; // Enlarge
    if (key == GLFW_KEY_R && action == GLFW_PRESS) mode = 2;
}