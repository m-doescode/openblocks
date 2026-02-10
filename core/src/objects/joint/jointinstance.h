#pragma once

#include "objectmodel/macro.h"
#include "objects/base/instance.h"
#include <memory>
#include "datatypes/cframe.h"
#include "physics/world.h"

class BasePart;
class Workspace;

class JointInstance : public Instance {
    INSTANCE_HEADER

    std::weak_ptr<BasePart> oldPart0;
    std::weak_ptr<BasePart> oldPart1;
protected:
    // The workspace the joint was created in, if it exists
    std::weak_ptr<Workspace> jointWorkspace;
    PhysJoint joint;

    void OnAncestryChanged(nullable std::shared_ptr<Instance>, nullable std::shared_ptr<Instance>) override;

    nullable std::shared_ptr<Workspace> workspaceOfPart(std::shared_ptr<BasePart>);
    inline void onUpdated(std::string property, Variant, Variant) { Update(); };

    virtual void buildJoint() = 0;
    virtual bool isDrivenJoint();
public:
    void Update();
    virtual void OnPartParamsUpdated();

    std::weak_ptr<BasePart> part0;
    std::weak_ptr<BasePart> part1;
    CFrame c0;
    CFrame c1;

    virtual void OnPhysicsStep(float deltaTime);

    JointInstance();
    ~JointInstance();
};

#undef DEF_PROP_PHYS