#pragma once

#include "objectmodel/macro.h"
#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

class Weld : public JointInstance {
    INSTANCE_HEADER

    virtual void buildJoint() override;
public:
    ~Weld();

    static inline std::shared_ptr<Weld> New() { return std::make_shared<Weld>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Weld>(); };
};