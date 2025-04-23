#pragma once

#include "objects/base/instance.h"
#include <memory>
#include <optional>

class Part;
class Workspace;

class JointInstance : public Instance {
    std::weak_ptr<Part> oldPart0;
    std::weak_ptr<Part> oldPart1;
protected:
    // The workspace the joint was created in, if it exists
    std::weak_ptr<Workspace> jointWorkspace;

    void OnAncestryChanged(std::optional<std::shared_ptr<Instance>>, std::optional<std::shared_ptr<Instance>>) override;

    std::optional<std::shared_ptr<Workspace>> workspaceOfPart(std::shared_ptr<Part>);
    void onUpdated(std::string property);
    virtual void buildJoint() = 0;
    virtual void breakJoint() = 0;
public:
    const static InstanceType TYPE;

    std::weak_ptr<Part> part0;
    std::weak_ptr<Part> part1;
    Data::CFrame c0;
    Data::CFrame c1;

    JointInstance(const InstanceType*);
    ~JointInstance();

    virtual const InstanceType* GetClass() override;
};