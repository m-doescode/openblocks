#pragma once

#include "objects/base/instance.h"
#include "../annotation.h"
#include <memory>
#include <optional>
#include "datatypes/cframe.h"

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

    void OnAncestryChanged(std::optional<std::shared_ptr<Instance>>, std::optional<std::shared_ptr<Instance>>) override;

    std::optional<std::shared_ptr<Workspace>> workspaceOfPart(std::shared_ptr<BasePart>);
    void onUpdated(std::string property);
    virtual void buildJoint() = 0;
    virtual void breakJoint() = 0;
public:

    DEF_PROP_(on_update=onUpdated) std::weak_ptr<BasePart> part0;
    DEF_PROP_(on_update=onUpdated) std::weak_ptr<BasePart> part1;
    DEF_PROP_(on_update=onUpdated) CFrame c0;
    DEF_PROP_(on_update=onUpdated) CFrame c1;

    JointInstance(const InstanceType*);
    ~JointInstance();
};