#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

class DEF_INST Rotate : public JointInstance {
    AUTOGEN_PREAMBLE

    virtual void buildJoint() override;
public:
    Rotate();
    ~Rotate();

    static inline std::shared_ptr<Rotate> New() { return std::make_shared<Rotate>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Rotate>(); };
};