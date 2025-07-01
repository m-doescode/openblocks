#include "rotate.h"
#include "objects/service/jointsservice.h"
#include "objects/part.h"
#include "objects/service/workspace.h"
#include "rendering/renderer.h"
#include <reactphysics3d/constraint/HingeJoint.h>

Rotate::Rotate(): JointInstance(&TYPE) {
}

Rotate::~Rotate() {
}
static CFrame XYZToZXY(glm::vec3(0, 0, 0), -glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

void Rotate::buildJoint() {
    // Only if both parts are set, are not the same part, are part of a workspace, and are part of the same workspace, we build the joint
    if (part0.expired() || part1.expired() || part0.lock() == part1.lock() || !workspaceOfPart(part0.lock()) || workspaceOfPart(part0.lock()) != workspaceOfPart(part1.lock())) return;

    // Don't build the joint if we're not part of either a workspace or JointsService
    if ((!GetParent() || GetParent().value()->GetClass() != &JointsService::TYPE) && !workspace()) return;

    std::shared_ptr<Workspace> workspace = workspaceOfPart(part0.lock()).value();

    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    CFrame newFrame = part0.lock()->cframe * (c0 * c1.Inverse());
    part1.lock()->cframe = newFrame;
    workspace->SyncPartPhysics(part1.lock());
    // Do NOT use Abs() in this scenario. For some reason that breaks it
    rp::HingeJointInfo jointInfo(part0.lock()->rigidBody, part1.lock()->rigidBody, (part0.lock()->cframe * c0).Position(), -(part0.lock()->cframe * c0).LookVector().Unit());
    this->joint = dynamic_cast<rp::HingeJoint*>(workspace->CreateJoint(jointInfo));
    jointWorkspace = workspace;

    // part1.lock()->rigidBody->getCollider(0)->setCollideWithMaskBits(0b10);
    // part1.lock()->rigidBody->getCollider(0)->setCollisionCategoryBits(0b10);
    // part0.lock()->rigidBody->getCollider(0)->setCollideWithMaskBits(0b01);
    // part0.lock()->rigidBody->getCollider(0)->setCollisionCategoryBits(0b01);
}

// !!! REMINDER: This has to be called manually when parts are destroyed/removed from the workspace, or joints will linger
void Rotate::breakJoint() {
    // If the joint doesn't exist, or its workspace expired (not our problem anymore), then no need to do anything
    if (!this->joint || jointWorkspace.expired()) return;

    jointWorkspace.lock()->DestroyJoint(this->joint);
    this->joint = nullptr;
}