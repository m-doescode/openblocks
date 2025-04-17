#pragma once

#include "objects/base/instance.h"
#include <memory>
#include <optional>

class Part;

class Snap : public Instance {
    rp::FixedJoint* joint;
protected:
    void OnWorkspaceAdded(std::optional<std::shared_ptr<Workspace>> oldWorkspace, std::shared_ptr<Workspace> newWorkspace) override;
    void OnWorkspaceRemoved(std::optional<std::shared_ptr<Workspace>> oldWorkspace) override;
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