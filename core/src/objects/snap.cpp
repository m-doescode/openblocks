#include "snap.h"

#include "datatypes/vector.h"
#include "workspace.h"
#include "part.h"
#include <reactphysics3d/constraint/FixedJoint.h>

const InstanceType Snap::TYPE = {
    .super = &Instance::TYPE,
    .className = "Snap",
    .constructor = &Snap::Create,
};

const InstanceType* Snap::GetClass() {
    return &TYPE;
}

Snap::Snap(): Instance(&TYPE) {
}

Snap::~Snap() {
}

void Snap::OnWorkspaceAdded(std::optional<std::shared_ptr<Workspace>> oldWorkspace, std::shared_ptr<Workspace> newWorkspace) {
    if (!part0 || !part1 || part0->expired() || part1->expired()) return;
    
    printVec((part0->lock()->cframe * (c1.Inverse() * c0)).Rotation().ToEulerAnglesXYZ());
    printVec(part1->lock()->cframe.Rotation().ToEulerAnglesXYZ());
    
    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    Data::CFrame newFrame = part0->lock()->cframe * (c1.Inverse() * c0);
    part1->lock()->cframe = newFrame;
    newWorkspace->SyncPartPhysics(part1->lock());

    rp::FixedJointInfo jointInfo(part0->lock()->rigidBody, part1->lock()->rigidBody, (c0.Inverse() * c1).Position());
    this->joint = dynamic_cast<rp::FixedJoint*>(workspace().value()->physicsWorld->createJoint(jointInfo));
}

void Snap::OnWorkspaceRemoved(std::optional<std::shared_ptr<Workspace>> oldWorkspace) {
    if (!this->joint || !oldWorkspace) return;

    oldWorkspace.value()->physicsWorld->destroyJoint(this->joint);
    this->joint = nullptr;
}