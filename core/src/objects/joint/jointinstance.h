#pragma once

#include "objects/base/instance.h"
#include "../annotation.h"
#include <memory>
#include <optional>
#include "datatypes/cframe.h"
#include "physics/world.h"

//this is necessary ebcause we use std::weak_ptr<Part> without including it in this file
#ifdef __AUTOGEN_EXTRA_INCLUDES__
#include "objects/part/part.h"
#endif

class BasePart;
class Workspace;

class DEF_INST_ABSTRACT JointInstance : public Instance {
    AUTOGEN_PREAMBLE

    std::weak_ptr<BasePart> oldPart0;
    std::weak_ptr<BasePart> oldPart1;
protected:
    // The workspace the joint was created in, if it exists
    std::weak_ptr<Workspace> jointWorkspace;
    PhysJoint joint;

    void OnAncestryChanged(nullable std::shared_ptr<Instance>, nullable std::shared_ptr<Instance>) override;

    nullable std::shared_ptr<Workspace> workspaceOfPart(std::shared_ptr<BasePart>);
    inline void onUpdated(std::string property) { Update(); };

    virtual void buildJoint() = 0;
public:
    void Update();

    DEF_PROP_(on_update=onUpdated) std::weak_ptr<BasePart> part0;
    DEF_PROP_(on_update=onUpdated) std::weak_ptr<BasePart> part1;
    DEF_PROP_(on_update=onUpdated) CFrame c0;
    DEF_PROP_(on_update=onUpdated) CFrame c1;

    JointInstance(const InstanceType*);
    ~JointInstance();
};