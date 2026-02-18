#pragma once
#include <glm/glm.hpp>

enum Direction {
    DIRECTION_FORWARD,
    DIRECTION_BACKWARDS,
    DIRECTION_RIGHT,
    DIRECTION_LEFT,
    DIRECTION_UP,
    DIRECTION_DOWN,
};

enum class CameraMode {
    FIRSTPERSON,
    ORBIT
};

class Camera {
public:
    glm::vec3 cameraPos;
    glm::vec3 targetPos;
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    CameraMode mode = CameraMode::FIRSTPERSON;

    float pitch = 0., yaw = -90., roll = 0.;

    float movementSpeed = 10.0f;
    float mouseSensitivity = 0.2f;

    Camera(glm::vec3 initialPosition);

    glm::mat4 getLookAt();
    /** Converts a set of screen coords to a direction from the camera's pos */
    glm::vec3 getScreenDirection(glm::vec2 screenPos, glm::vec2 screenSize);
    void processRotation(float deltaX, float deltaY);
    void processMovement(Direction direction, float deltaTime);
};
