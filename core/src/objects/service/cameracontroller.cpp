
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
    float zoomSpeed = this->zoomSpeed * deltaTime;

    glm::vec3 targetPos = camera->focus.Position();
    glm::vec3 cameraFront = camera->cframe.LookVector();
    glm::vec3 cameraUp(0, 1, 0);
    Vector3 destPos;

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
        
        case Direction::IN:
        
            // Handle case of user zooming all the way in
            if ((camera->cframe.Position() + camera->cframe.LookVector() - camera->focus.Position()).Dot(camera->cframe.LookVector()) > 0) {
                break;
            }
            
            camera->cframe = camera->cframe.Position() + camera->cframe.LookVector() * zoomSpeed;
            break;
        case Direction::OUT:
            camera->cframe = camera->cframe.Position() - camera->cframe.LookVector() * zoomSpeed;
            break;
    }

    // TODO: Add third person orbiting as well
    camera->cframe = camera->cframe.Rotation() + (camera->cframe.Position() - camera->focus.Position()) + targetPos;
    camera->focus = camera->focus.Rotation() + targetPos;
    camera->UpdateView();
}

void CameraController::InputRotation(float deltaX, float deltaY) {
    auto workspace = dataModel()->GetService<Workspace>();
    auto camera = workspace->GetCamera();

    deltaX *= this->mouseSensitivity;
    deltaY *= this->mouseSensitivity;

    camera->UpdateView();
    float dist = (camera->cframe.Position() - camera->focus.Position()).Magnitude();
    float pitch = camera->pitch, yaw = camera->yaw;

    yaw += -deltaX;
    pitch += -deltaY;

    // Prevent world flipping if pitch exceeds 90deg
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    camera->pitch = pitch, camera->yaw = yaw;
    Vector3 eulerAngles = Vector3(deg2rad(pitch), deg2rad(yaw), 0);
    CFrame angledFrame = CFrame::FromEulerAnglesYXZ(eulerAngles);

    Vector3 pos = camera->focus.Position() - angledFrame.LookVector() * dist;

    camera->cframe = CFrame(pos, camera->focus.Position(), Vector3(0, 1, 0));
    camera->UpdateView();
}
