#include "camera.h"
#include <glm/ext/matrix_clip_space.hpp>
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

    if (mode == CameraMode::FIRSTPERSON) {
        cameraPos = targetPos;
    } else if (mode == CameraMode::ORBIT) {
        cameraPos = targetPos + cameraFront * 10.f;
        cameraFront = -cameraFront;
    }

    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::processMovement(Direction direction, float deltaTime) {
    float speed = this->movementSpeed * deltaTime;

    switch (direction) {
        case DIRECTION_FORWARD:
            targetPos += speed * cameraFront;
            break;
        case DIRECTION_BACKWARDS:
            targetPos -= speed * cameraFront;
            break;
        case DIRECTION_LEFT:
            targetPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
            break;
        case DIRECTION_RIGHT:
            targetPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
            break;
        case DIRECTION_UP:
            targetPos += cameraUp * speed;
            break;
        case DIRECTION_DOWN:
            targetPos -= cameraUp * speed;
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

glm::vec3 Camera::getScreenDirection(glm::vec2 screenPos, glm::vec2 screenSize) {
    // VVV Thank goodness for this person's answer
    // https://stackoverflow.com/a/30005258/16255372

    // glm::vec3 worldPos = camera.cameraPos + glm::vec3(glm::vec4(float(position.x()) / width() - 0.5f, float(position.y()) / height() - 0.5f, 0, 0) * camera.getLookAt());
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)screenSize.x / (float)screenSize.y, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0), this->cameraFront, this->cameraUp);
    glm::mat4 inverseViewport = glm::inverse(projection * view);

    glm::vec2 ndc = glm::vec2(screenPos.x / screenSize.x * 2.f - 1.f, -screenPos.y / screenSize.y * 2.f + 1.f);
    glm::vec4 world = glm::normalize(inverseViewport * glm::vec4(ndc, 1, 1));
    //glm::vec3 flat = glm::vec3(world) / world.w; // https://stackoverflow.com/a/68870587/16255372

    return glm::vec3(world);
}