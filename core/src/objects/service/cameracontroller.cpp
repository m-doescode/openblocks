
#include "cameracontroller.h"
#include "datatypes/vector.h"
#include "math_helper.h"
#include "objectmodel/type.h"
#include "objects/datamodel.h"
#include "objects/service/workspace.h"

INSTANCE_IMPL(CameraController)

InstanceType CameraController::__buildType() {
    return make_instance_type<CameraController>("CameraController", INSTANCE_HIDDEN);
}

void CameraController::InitService() {
    if (initialized) return;
    initialized = true;
}

void CameraController::InputMovement(Direction direction, float deltaTime) {
    auto workspace = dataModel()->GetService<Workspace>();
    auto camera = workspace->GetCamera();

    float speed = this->movementSpeed * deltaTime;

    glm::vec3 targetPos = camera->cframe.Position();
    glm::vec3 cameraFront = camera->cframe.LookVector();
    glm::vec3 cameraUp(0, 1, 0);

    switch (direction) {
        case Direction::FORWARD:
            targetPos += speed * cameraFront;
            break;
        case Direction::BACKWARDS:
            targetPos -= speed * cameraFront;
            break;
        case Direction::LEFT:
            targetPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
            break;
        case Direction::RIGHT:
            targetPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
            break;
        case Direction::UP:
            targetPos += cameraUp * speed;
            break;
        case Direction::DOWN:
            targetPos -= cameraUp * speed;
            break;
    }

    // TODO: Add third person orbiting as well
    camera->cframe = camera->cframe.Rotation() + targetPos;
}

static CFrame fromFirstPersonAngles(Vector3 pos, float pitch, float yaw) {
    glm::vec3 direction, cameraFront;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    return CFrame(pos, pos + cameraFront, glm::vec3(0, 1, 0));
}

void CameraController::InputRotation(float deltaX, float deltaY) {
    auto workspace = dataModel()->GetService<Workspace>();
    auto camera = workspace->GetCamera();

    deltaX *= this->mouseSensitivity;
    deltaY *= this->mouseSensitivity;

    Vector3 eulerAngles = camera->cframe.ToEulerAnglesZXY();
    float yaw = rad2deg(eulerAngles.Y()), pitch = rad2deg(eulerAngles.X());

    yaw += deltaX;
    pitch += deltaY;

    // Prevent world flipping if pitch exceeds 90deg
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    eulerAngles = Vector3(deg2rad(pitch), deg2rad(yaw), 0);
    camera->cframe = CFrame::FromEulerAnglesZXY(eulerAngles) + camera->cframe.Position();
    lastCFrame = camera->cframe;
}
