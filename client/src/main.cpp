#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <stdio.h>

#include "objects/part.h"
#include "rendering/renderer.h"
#include "physics/simulation.h"
#include "camera.h"

#include "common.h"

void errorCatcher(int id, const char* str);

int mode = 0;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
// void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void resizeCallback(GLFWwindow* window, int width, int height);

std::shared_ptr<Part> lastPart;

int main() {
    glfwSetErrorCallback(errorCatcher);

    glfwInit();
    GLFWwindow *window = glfwCreateWindow(1200, 900, "OpenBlocks Client ALPHA", NULL, NULL);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    glfwMakeContextCurrent(window);
    glewInit();

    dataModel->Init();
    simulationInit();
    renderInit(window, 1200, 900);

    // Baseplate
    workspace()->AddChild(Part::New({
        .position = glm::vec3(0, -5, 0),
        .rotation = glm::vec3(0),
        .scale = glm::vec3(512, 1.2, 512),
        .material = Material {
            .diffuse = glm::vec3(0.388235, 0.372549, 0.384314),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 32.0f,
        },
        .anchored = true,
    }));

    workspace()->AddChild(lastPart = Part::New({
        .position = glm::vec3(0),
        .rotation = glm::vec3(0),
        .scale = glm::vec3(4, 1.2, 2),
        .material = Material {
            .diffuse = glm::vec3(0.639216f, 0.635294f, 0.647059f),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 32.0f,
        }
    }));

    for (InstanceRef inst : workspace()->GetChildren()) {
        if (inst->GetClass()->className != "Part") continue;
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst);
        syncPartPhysics(part);
    }

    float lastTime = glfwGetTime();
    do {
        float deltaTime = glfwGetTime() - lastTime;
        lastTime = glfwGetTime();
        
        processInput(window);
        physicsStep(deltaTime);
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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processMovement(DIRECTION_UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.processMovement(DIRECTION_DOWN, deltaTime);

    if (mode == 2) {
        float shiftFactor = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? -0.5 : 0.5;
        shiftFactor *= deltaTime;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            lastPart->rotation *= glm::angleAxis(shiftFactor, glm::vec3(1, 0, 0));
            syncPartPhysics(lastPart);
        }
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
            lastPart->rotation *= glm::angleAxis(shiftFactor, glm::vec3(0, 1, 0));
            syncPartPhysics(lastPart);
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            lastPart->rotation *= glm::angleAxis(shiftFactor, glm::vec3(0, 0, 1));
            syncPartPhysics(lastPart);
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
        workspace()->AddChild(lastPart = Part::New({
            .position = camera.cameraPos + camera.cameraFront * glm::vec3(3),
            .rotation = glm::vec3(0),
            .scale = glm::vec3(1, 1, 1),
            .material = Material {
                .diffuse = glm::vec3(1.0f, 0.5f, 0.31f),
                .specular = glm::vec3(0.5f, 0.5f, 0.5f),
                .shininess = 32.0f,
            }
        }));
        syncPartPhysics(lastPart);
    }

    float shiftFactor = (mods & GLFW_MOD_SHIFT) ? -0.2 : 0.2;
    if (mode == 0) {
        if (key == GLFW_KEY_X && action == GLFW_PRESS) {
            lastPart->position.x += shiftFactor;
            syncPartPhysics(lastPart);
        }
        if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
            lastPart->position.y += shiftFactor;
            syncPartPhysics(lastPart);
        }
        if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
            lastPart->position.z += shiftFactor;
            syncPartPhysics(lastPart);
        }
    } else if (mode == 1) {
        if (key == GLFW_KEY_X && action == GLFW_PRESS) {
            lastPart->scale.x += shiftFactor;
            syncPartPhysics(lastPart);
        }
        if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
            lastPart->scale.y += shiftFactor;
            syncPartPhysics(lastPart);
        }
        if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
            lastPart->scale.z += shiftFactor;
            syncPartPhysics(lastPart);
        }
    }
    
    if (key == GLFW_KEY_M && action == GLFW_PRESS) mode = 0;
    if (key == GLFW_KEY_E && action == GLFW_PRESS) mode = 1; // Enlarge
    if (key == GLFW_KEY_R && action == GLFW_PRESS) mode = 2;
}

void resizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    setViewport(width, height);
}