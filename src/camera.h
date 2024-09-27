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

class Camera {
public:
    glm::vec3 cameraPos;
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float pitch = 0., yaw = -90., roll = 0.;

    float movementSpeed = 2.5f;
    float mouseSensitivity = 0.2f;

    Camera(glm::vec3 initialPosition);

    glm::mat4 getLookAt();
    void processRotation(float deltaX, float deltaY);
    void processMovement(Direction direction, float deltaTime);

};
