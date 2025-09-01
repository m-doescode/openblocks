#include "jointinstance.h"

#include "datatypes/ref.h"
#include "objects/datamodel.h"
#include "objects/service/jointsservice.h"
#include "objects/part/basepart.h"
#include "objects/service/workspace.h"
#include <memory>

JointInstance::JointInstance(const InstanceType* type): Instance(type) {
}

JointInstance::~JointInstance() {
}

void JointInstance::OnAncestryChanged(nullable std::shared_ptr<Instance>, nullable std::shared_ptr<Instance>) {
    Update();
}

void JointInstance::OnPartParamsUpdated() {
}

void JointInstance::Update() {
    // To keep it simple compared to our previous algorithm, this one is pretty barebones:
    // 1. Every time we update, (whether our parent changed, or a property), destroy the current joints
    // 2. If the new configuration is valid, rebuild our joints

    if (!jointWorkspace.expired()) {
        jointWorkspace.lock()->DestroyJoint(joint);
        if (!oldPart0.expired())
            oldPart0.lock()->untrackJoint(shared<JointInstance>());
        if (!oldPart1.expired())
            oldPart1.lock()->untrackJoint(shared<JointInstance>());
    }

    oldPart0 = part0;
    oldPart1 = part1;

    // Don't build the joint if we're not part of either a workspace or JointsService
    if ((!GetParent() || GetParent()->GetClass() != &JointsService::TYPE) && !workspace()) return;

    // If either part is invalid or they are part of separate worlds, fail
    if (part0.expired()
        || part1.expired()
        || !workspaceOfPart(part0.lock())
        || !workspaceOfPart(part1.lock())
        || workspaceOfPart(part0.lock()) != workspaceOfPart(part1.lock())
    ) return;

    // TODO: Add joint continuity check here

    // Finally, build the joint
    buildJoint();

    part0.lock()->trackJoint(shared<JointInstance>());
    part1.lock()->trackJoint(shared<JointInstance>());
}

nullable std::shared_ptr<Workspace> JointInstance::workspaceOfPart(std::shared_ptr<BasePart> part) {
    return part->workspace();
}