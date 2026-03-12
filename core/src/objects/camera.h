#pragma once

#include <memory>
#include "objectmodel/macro.h"
#include "objects/base/instance.h"

class CameraController;

class Camera : public Instance {
    INSTANCE_HEADER

    friend CameraController;

    void onChanged(std::string name, Variant oldValue, Variant newValue);
    float pitch = 0, yaw = 0;
public:
    enum class Mode {
        FirstPerson,
        Orbit
    };

    static inline std::shared_ptr<Camera> New() { return new_instance<Camera>(); };
    static inline std::shared_ptr<Instance> Create() { return new_instance<Camera>(); };

    void UpdateView();

    CFrame focus = CFrame(Vector3(0, 0, 0));
    CFrame cframe = CFrame(Vector3(0, 0, 0.0002));
    std::weak_ptr<Instance> cameraSubject;
    float fieldOfView = 45.f; // vertical degrees
    Mode mode = Mode::FirstPerson;

    /** Converts a set of screen coords to a direction from the camera's pos */
    Vector3 GetScreenDirection(glm::vec2 screenPos, glm::vec2 screenSize);

    // Temporary function to help with errors caused by my own lack of math understanding
    glm::mat4 getCameraLookAt();
};