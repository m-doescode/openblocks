#include "camera.h"
#include <glm/ext/matrix_transform.hpp>

Camera::Camera(glm::vec3 initalPosition) {
    this->cameraPos = initalPosition;
}

glm::mat4 Camera::getLookAt() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::processMovement(Direction direction, float deltaTime) {
    float speed = this->movementSpeed * deltaTime;

    switch (direction) {
        case DIRECTION_FORWARD:
            cameraPos += speed * cameraFront;
            break;
        case DIRECTION_BACKWARDS:
            cameraPos -= speed * cameraFront;
            break;
        case DIRECTION_LEFT:
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
            break;
        case DIRECTION_RIGHT:
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
            break;
        case DIRECTION_UP:
            cameraPos += cameraUp * speed;
            break;
        case DIRECTION_DOWN:
            cameraPos -= cameraUp * speed;
            break;
    }
}

void Camera::processRotation(float deltaX, float deltaY) {
    deltaX *= this->mouseSensitivity;
    deltaY *= this->mouseSensitivity;

    yaw += deltaX;
    pitch += -deltaY;

    // Prevent world flipping if pitch exceeds 90deg
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;
}
