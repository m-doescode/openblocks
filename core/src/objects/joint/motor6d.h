#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

#define DEF_PROP_PHYS DEF_PROP_(on_update=onUpdated)

class DEF_INST Motor6D : public JointInstance {
    AUTOGEN_PREAMBLE

    virtual void buildJoint() override;
    void onUpdated(std::string);
public:
    Motor6D();
    ~Motor6D();

    DEF_PROP_PHYS float desiredAngle;
    DEF_PROP_PHYS float maxVelocity;

    static inline std::shared_ptr<Motor6D> New() { return std::make_shared<Motor6D>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Motor6D>(); };
};

#undef DEF_PROP_PHYS