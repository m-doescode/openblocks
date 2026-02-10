#pragma once

#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

class Rotate : public JointInstance {
    INSTANCE_HEADER

    virtual void buildJoint() override;
public:
    Rotate();
    ~Rotate();

    static inline std::shared_ptr<Rotate> New() { return std::make_shared<Rotate>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Rotate>(); };
};