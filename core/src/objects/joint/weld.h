#pragma once

#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

class Weld : public JointInstance {
    rp::FixedJoint* joint = nullptr;

    virtual void buildJoint() override;
    virtual void breakJoint() override;
public:
    const static InstanceType TYPE;

    Weld();
    ~Weld();

    static inline std::shared_ptr<Weld> New() { return std::make_shared<Weld>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Weld>(); };
    virtual const InstanceType* GetClass() override;
};