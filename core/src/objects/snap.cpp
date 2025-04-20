#include "snap.h"

#include "datatypes/cframe.h"
#include "datatypes/ref.h"
#include "objects/datamodel.h"
#include "objects/jointsservice.h"
#include "workspace.h"
#include "part.h"
#include <memory>
#include <reactphysics3d/constraint/FixedJoint.h>
#include <reactphysics3d/engine/PhysicsWorld.h>

const InstanceType Snap::TYPE = {
    .super = &Instance::TYPE,
    .className = "Snap",
    .constructor = &Snap::Create,
};

const InstanceType* Snap::GetClass() {
    return &TYPE;
}

Snap::Snap(): Instance(&TYPE) {
    this->memberMap = std::make_unique<MemberMap>(MemberMap {
        .super = std::move(this->memberMap),
        .members = {
            { "Part0", {
                .backingField = &part0,
                .type = &Data::InstanceRef::TYPE,
                .codec = fieldCodecOf<Data::InstanceRef, std::weak_ptr<Instance>>(),
                .updateCallback = memberFunctionOf(&Snap::onUpdated, this),
            }}, { "Part1", {
                .backingField = &part1,
                .type = &Data::InstanceRef::TYPE,
                .codec = fieldCodecOf<Data::InstanceRef, std::weak_ptr<Instance>>(),
                .updateCallback = memberFunctionOf(&Snap::onUpdated, this),
            }}, { "C0", {
                .backingField = &c0,
                .type = &Data::CFrame::TYPE,
                .codec = fieldCodecOf<Data::CFrame>(),
                .updateCallback = memberFunctionOf(&Snap::onUpdated, this),
            }}, { "C1", {
                .backingField = &c0,
                .type = &Data::CFrame::TYPE,
                .codec = fieldCodecOf<Data::CFrame>(),
                .updateCallback = memberFunctionOf(&Snap::onUpdated, this),
            }}, 
        }
    });
}

Snap::~Snap() {
}

void Snap::OnAncestryChanged(std::optional<std::shared_ptr<Instance>>, std::optional<std::shared_ptr<Instance>>) {
    // Destroy and rebuild the joint, it's the simplest solution that actually works

    breakJoint();
    buildJoint();
}

void Snap::onUpdated(std::string property) {
    // Destroy and rebuild the joint, if applicable

    breakJoint();
    buildJoint();
}

void Snap::buildJoint() {
    // Only if both parts are set, are not the same part, are part of a workspace, and are part of the same workspace, we build the joint
    if (part0.expired() || part1.expired() || part0.lock() == part1.lock() || !part0.lock()->workspace() || part0.lock()->workspace() != part1.lock()->workspace()) return;

    // Don't build the joint if we're not part of either a workspace or JointsService
    if ((!GetParent() || GetParent().value()->GetClass() != &JointsService::TYPE) && !workspace()) return;

    std::shared_ptr<Workspace> workspace = part0.lock()->workspace().value();
    if (!workspace->physicsWorld) return;

    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    Data::CFrame newFrame = part0.lock()->cframe * (c1.Inverse() * c0);
    part1.lock()->cframe = newFrame;
    workspace->SyncPartPhysics(part1.lock());

    rp::FixedJointInfo jointInfo(part0.lock()->rigidBody, part1.lock()->rigidBody, (c0.Inverse() * c1).Position());
    this->joint = dynamic_cast<rp::FixedJoint*>(workspace->physicsWorld->createJoint(jointInfo));
    jointWorkspace = workspace;
}

// !!! REMINDER: This has to be called manually when parts are destroyed/removed from the workspace, or joints will linger
void Snap::breakJoint() {
    // If the joint doesn't exist, or its workspace expired (not our problem anymore), then no need to do anything
    if (!this->joint || jointWorkspace.expired() || !jointWorkspace.lock()->physicsWorld) return;

    jointWorkspace.lock()->physicsWorld->destroyJoint(this->joint);
    this->joint = nullptr;
}