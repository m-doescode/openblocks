#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

class DEF_INST Snap : public JointInstance {
    AUTOGEN_PREAMBLE

    virtual void buildJoint() override;
public:
    Snap();
    ~Snap();

    static inline std::shared_ptr<Snap> New() { return std::make_shared<Snap>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Snap>(); };
};