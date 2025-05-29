#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "objects/part.h"
#include "rendering/renderer.h"
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
    Logger::init();

    glfwSetErrorCallback(errorCatcher);

    glfwInit();
    GLFWwindow *window = glfwCreateWindow(1200, 900, "OpenBlocks Client ALPHA", NULL, NULL);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    glfwMakeContextCurrent(window);
    glewInit();

    gDataModel->Init();
    renderInit(window, 1200, 900);

    // Baseplate
    gWorkspace()->AddChild(Part::New({
        .position = glm::vec3(0, -5, 0),
        .rotation = glm::vec3(0),
        .size = glm::vec3(512, 1.2, 512),
        .color = glm::vec3(0.388235, 0.372549, 0.384314),
        .anchored = true,
        .locked = true,
    }));

    gWorkspace()->AddChild(lastPart = Part::New({
        .position = glm::vec3(0),
        .rotation = glm::vec3(0),
        .size = glm::vec3(4, 1.2, 2),
        .color = glm::vec3(0.639216f, 0.635294f, 0.647059f),
    }));

    for (std::shared_ptr<Instance> inst : gWorkspace()->GetChildren()) {
        if (inst->GetClass()->className != "Part") continue;
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst);
        gWorkspace()->SyncPartPhysics(part);
    }

    float lastTime = glfwGetTime();
    do {
        float deltaTime = glfwGetTime() - lastTime;
        lastTime = glfwGetTime();
        
        processInput(window);
        gWorkspace()->PhysicsStep(deltaTime);
        render(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(!glfwWindowShouldClose(window));

    glfwTerminate();
    return 0;
}

void errorCatcher(int id, const char* str) {
    Logger::fatalError(std::format("GLFW Error: [{}] {}", id, str));
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
            // lastPart->rotation *= glm::angleAxis(shiftFactor, glm::vec3(1, 0, 0));
            gWorkspace()->SyncPartPhysics(lastPart);
        }
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
            // lastPart->rotation *= glm::angleAxis(shiftFactor, glm::vec3(0, 1, 0));
            gWorkspace()->SyncPartPhysics(lastPart);
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            // lastPart->rotation *= glm::angleAxis(shiftFactor, glm::vec3(0, 0, 1));
            gWorkspace()->SyncPartPhysics(lastPart);
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
        gWorkspace()->AddChild(lastPart = Part::New({
            .position = camera.cameraPos + camera.cameraFront * glm::vec3(3),
            .rotation = glm::vec3(0),
            .size = glm::vec3(1, 1, 1),
            .color = glm::vec3(1.0f, 0.5f, 0.31f),
        }));
        gWorkspace()->SyncPartPhysics(lastPart);
    }

    float shiftFactor = (mods & GLFW_MOD_SHIFT) ? -0.2 : 0.2;
    if (mode == 0) {
        if (key == GLFW_KEY_X && action == GLFW_PRESS) {
            // lastPart->position.x += shiftFactor;
            gWorkspace()->SyncPartPhysics(lastPart);
        }
        if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
            // lastPart->position.y += shiftFactor;
            gWorkspace()->SyncPartPhysics(lastPart);
        }
        if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
            // lastPart->position.z += shiftFactor;
            gWorkspace()->SyncPartPhysics(lastPart);
        }
    } else if (mode == 1) {
        if (key == GLFW_KEY_X && action == GLFW_PRESS) {
            lastPart->size.x += shiftFactor;
            gWorkspace()->SyncPartPhysics(lastPart);
        }
        if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
            lastPart->size.y += shiftFactor;
            gWorkspace()->SyncPartPhysics(lastPart);
        }
        if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
            lastPart->size.z += shiftFactor;
            gWorkspace()->SyncPartPhysics(lastPart);
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