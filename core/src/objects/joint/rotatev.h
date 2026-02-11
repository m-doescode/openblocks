#pragma once

#include "objects/base/instance.h"
#include "objects/joint/jointinstance.h"
#include <memory>

class RotateV : public JointInstance {
    INSTANCE_HEADER

    virtual void buildJoint() override;
public:
    ~RotateV();

    void OnPartParamsUpdated() override;

    static inline std::shared_ptr<RotateV> New() { return new_instance<RotateV>(); };
    static inline std::shared_ptr<Instance> Create() { return new_instance<RotateV>(); };
};