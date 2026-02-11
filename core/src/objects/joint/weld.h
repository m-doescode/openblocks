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

    static inline std::shared_ptr<Weld> New() { return new_instance<Weld>(); };
    static inline std::shared_ptr<Instance> Create() { return new_instance<Weld>(); };
};