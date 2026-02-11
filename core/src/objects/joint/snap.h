#pragma once

#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

class Snap : public JointInstance {
    INSTANCE_HEADER

    virtual void buildJoint() override;
public:
    ~Snap();

    static inline std::shared_ptr<Snap> New() { return std::make_shared<Snap>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Snap>(); };
};