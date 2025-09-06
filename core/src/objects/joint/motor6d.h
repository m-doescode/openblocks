#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

class DEF_INST Motor6D : public JointInstance {
    AUTOGEN_PREAMBLE

    virtual void buildJoint() override;
    void onUpdated(std::string);

    void OnPhysicsStep(float deltaTime) override;
    bool isDrivenJoint() override;
public:
    Motor6D();
    ~Motor6D();

    DEF_PROP float currentAngle = 0;
    DEF_PROP float desiredAngle = 0;
    DEF_PROP float maxVelocity = 0.1;

    static inline std::shared_ptr<Motor6D> New() { return std::make_shared<Motor6D>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Motor6D>(); };
};