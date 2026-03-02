#pragma once

#include "objectmodel/macro.h"
#include "objects/base/service.h"

class CameraController : public Service {
    INSTANCE_HEADER
public:
    enum class Direction {
        FORWARD,
        BACKWARDS,
        RIGHT,
        LEFT,
        UP,
        DOWN,
    };
protected:
    void InitService() override;
    bool initialized = false;
public:
    static inline std::shared_ptr<Instance> Create() { return new_instance<CameraController>(); };

    float movementSpeed = 10.0f;
    float mouseSensitivity = 0.2f;

    void InputRotation(float deltaX, float deltaY);
    void InputMovement(Direction direction, float deltaTime);
};