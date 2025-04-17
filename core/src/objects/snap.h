#pragma once

#include "objects/base/instance.h"
#include <memory>
#include <optional>

class Part;
class Workspace;

class Snap : public Instance {
    rp::FixedJoint* joint = nullptr;

    std::weak_ptr<Instance> oldParent;
    // The actual workspace the joint is a part of
    std::weak_ptr<Workspace> oldWorkspace;
    // The pseudo-workspace the joint is a part of (including if parented to JointsService)
    std::weak_ptr<Workspace> oldJointWorkspace;
protected:
    void OnAncestryChanged(std::optional<std::shared_ptr<Instance>>, std::optional<std::shared_ptr<Instance>>) override;

    std::optional<std::shared_ptr<Workspace>> jointWorkspace();
    void onUpdated(std::string property);
    void buildJoint();
    void breakJoint();
public:
    const static InstanceType TYPE;

    std::weak_ptr<Part> part0;
    std::weak_ptr<Part> part1;
    Data::CFrame c0;
    Data::CFrame c1;

    Snap();
    ~Snap();

    static inline std::shared_ptr<Snap> New() { return std::make_shared<Snap>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Snap>(); };
    virtual const InstanceType* GetClass() override;
};