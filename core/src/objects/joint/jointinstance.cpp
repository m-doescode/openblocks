#include "jointinstance.h"

#include "datatypes/cframe.h"
#include "datatypes/ref.h"
#include "objects/datamodel.h"
#include "objects/service/jointsservice.h"
#include "objects/part/part.h"
#include "objects/service/workspace.h"
#include <memory>
#include <reactphysics3d/constraint/FixedJoint.h>
#include <reactphysics3d/engine/PhysicsWorld.h>
#include "ptr_helpers.h"

JointInstance::JointInstance(const InstanceType* type): Instance(type) {
}

JointInstance::~JointInstance() {
}

void JointInstance::OnAncestryChanged(nullable std::shared_ptr<Instance>, nullable std::shared_ptr<Instance>) {
    // Destroy and rebuild the joint, it's the simplest solution that actually works

    breakJoint();
    buildJoint();
}

void JointInstance::onUpdated(std::string property) {
    // Add ourselves to the attached parts, or remove, if applicable

    // Parts differ, delete 
    if (part0 != oldPart0 && !oldPart0.expired()) {
        oldPart0.lock()->untrackJoint(shared<JointInstance>());
    }

    if (part1 != oldPart1 && !oldPart1.expired()) {
        oldPart1.lock()->untrackJoint(shared<JointInstance>());
    }

    // Parts differ, add 
    if (part0 != oldPart0 && !part0.expired()) {
        part0.lock()->trackJoint(shared<JointInstance>());
    }

    if (part1 != oldPart1 && !part1.expired()) {
        part1.lock()->trackJoint(shared<JointInstance>());
    }

    // Destroy and rebuild the joint, if applicable

    breakJoint();
    buildJoint();

    oldPart0 = part0;
    oldPart1 = part1;
}

nullable std::shared_ptr<Workspace> JointInstance::workspaceOfPart(std::shared_ptr<BasePart> part) {
    return part->workspace();
}