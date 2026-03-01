#pragma once

#include <memory>
#include "objectmodel/macro.h"
#include "objects/base/instance.h"

class Camera : public Instance {
    INSTANCE_HEADER

    void onChanged();
    float roll, pitch, yaw;
public:
    static inline std::shared_ptr<Camera> New() { return new_instance<Camera>(); };
    static inline std::shared_ptr<Instance> Create() { return new_instance<Camera>(); };

    CFrame cframe;
    std::weak_ptr<Instance> cameraSubject;
    float fieldOfView = 45.f; // vertical degrees
};