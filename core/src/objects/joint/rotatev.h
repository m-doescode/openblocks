#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

class DEF_INST RotateV : public JointInstance {
    AUTOGEN_PREAMBLE

    virtual void buildJoint() override;
public:
    RotateV();
    ~RotateV();

    static inline std::shared_ptr<RotateV> New() { return std::make_shared<RotateV>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<RotateV>(); };
};