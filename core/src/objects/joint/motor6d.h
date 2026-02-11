#pragma once

#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

class Motor6D : public JointInstance {
    INSTANCE_HEADER

    virtual void buildJoint() override;
    void onUpdated(std::string);

    void OnPhysicsStep(float deltaTime) override;
    bool isDrivenJoint() override;
public:
    ~Motor6D();

    float currentAngle = 0;
    float desiredAngle = 0;
    float maxVelocity = 0.1;

    static inline std::shared_ptr<Motor6D> New() { return std::make_shared<Motor6D>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Motor6D>(); };
};