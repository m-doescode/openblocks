#pragma once

#include <memory>
#include "objectmodel/macro.h"
#include "objects/base/instance.h"

class CameraController;

class Camera : public Instance {
    INSTANCE_HEADER

    friend CameraController;

    void onChanged();
    // float roll, pitch, yaw;
public:
    static inline std::shared_ptr<Camera> New() { return new_instance<Camera>(); };
    static inline std::shared_ptr<Instance> Create() { return new_instance<Camera>(); };

    CFrame cframe;
    std::weak_ptr<Instance> cameraSubject;
    float fieldOfView = 45.f; // vertical degrees

    /** Converts a set of screen coords to a direction from the camera's pos */
    Vector3 GetScreenDirection(glm::vec2 screenPos, glm::vec2 screenSize);
};