#include "snap.h"

#include "datatypes/cframe.h"
#include "objects/datamodel.h"
#include "objects/joint/jointinstance.h"
#include "objects/service/jointsservice.h"
#include "objects/part/part.h"
#include "objects/service/workspace.h"
#include <memory>
#include <reactphysics3d/constraint/FixedJoint.h>
#include <reactphysics3d/engine/PhysicsWorld.h>

Snap::Snap(): JointInstance(&TYPE) {
}

Snap::~Snap() {
}

void Snap::buildJoint() {
    // Only if both parts are set, are not the same part, are part of a workspace, and are part of the same workspace, we build the joint
    if (part0.expired() || part1.expired() || part0.lock() == part1.lock() || !workspaceOfPart(part0.lock()) || workspaceOfPart(part0.lock()) != workspaceOfPart(part1.lock())) return;

    // Don't build the joint if we're not part of either a workspace or JointsService
    if ((!GetParent() || GetParent()->GetClass() != &JointsService::TYPE) && !workspace()) return;

    std::shared_ptr<Workspace> workspace = workspaceOfPart(part0.lock());

    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    CFrame newFrame = part0.lock()->cframe * (c0 * c1.Inverse());
    part1.lock()->cframe = newFrame;
    workspace->SyncPartPhysics(part1.lock());

    rp::FixedJointInfo jointInfo(part0.lock()->rigidBody, part1.lock()->rigidBody, (c0.Inverse() * c1).Position());
    this->joint = dynamic_cast<rp::FixedJoint*>(workspace->CreateJoint(jointInfo));
    jointWorkspace = workspace;
}

// !!! REMINDER: This has to be called manually when parts are destroyed/removed from the workspace, or joints will linger
void Snap::breakJoint() {
    // If the joint doesn't exist, or its workspace expired (not our problem anymore), then no need to do anything
    if (!this->joint || jointWorkspace.expired()) return;

    jointWorkspace.lock()->DestroyJoint(this->joint);
    this->joint = nullptr;
}