#pragma once

#include "objects/base/instance.h"
#include "../annotation.h"
#include <memory>
#include <optional>

//this is necessary ebcause we use std::weak_ptr<Part> without including it in this file
#ifdef __AUTOGEN_EXTRA_INCLUDES__
#include "../part.h"
#endif

class Part;
class Workspace;

class INSTANCE_WITH(abstract) JointInstance : public Instance {
    AUTOGEN_PREAMBLE

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

    [[ def_prop(name="Part0", on_update=onUpdated) ]]
    std::weak_ptr<Part> part0;
    [[ def_prop(name="Part1", on_update=onUpdated) ]]
    std::weak_ptr<Part> part1;
    [[ def_prop(name="C0", on_update=onUpdated) ]]
    CFrame c0;
    [[ def_prop(name="C1", on_update=onUpdated) ]]
    CFrame c1;

    JointInstance(const InstanceType*);
    ~JointInstance();
};