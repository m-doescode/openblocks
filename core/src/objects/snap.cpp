#include "snap.h"

#include "datatypes/cframe.h"
#include "datatypes/ref.h"
#include "datatypes/vector.h"
#include "workspace.h"
#include "part.h"
#include <memory>
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

void Snap::OnWorkspaceAdded(std::optional<std::shared_ptr<Workspace>> oldWorkspace, std::shared_ptr<Workspace> newWorkspace) {
    // Remove the existing joint if it does
    if (this->joint && oldWorkspace) {
        oldWorkspace.value()->physicsWorld->destroyJoint(this->joint);
        this->joint = nullptr;
    }

    buildJoint();
}

void Snap::OnWorkspaceRemoved(std::shared_ptr<Workspace> oldWorkspace) {
    if (!this->joint) return;

    oldWorkspace->physicsWorld->destroyJoint(this->joint);
    this->joint = nullptr;
}

void Snap::onUpdated(std::string property) {
    // We are not in the workspace, so we don't really care what values are currently set
    if (!workspace()) return;

    // Workspace cannot have changed, so if the joint currently exists, it is in the present one
    if (this->joint)
        workspace().value()->physicsWorld->destroyJoint(this->joint);

    buildJoint();
}

void Snap::buildJoint() {
    if (part0.expired() || part1.expired() || !workspace()) return;
    
    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    Data::CFrame newFrame = part0.lock()->cframe * (c1.Inverse() * c0);
    part1.lock()->cframe = newFrame;
    workspace().value()->SyncPartPhysics(part1.lock());

    rp::FixedJointInfo jointInfo(part0.lock()->rigidBody, part1.lock()->rigidBody, (c0.Inverse() * c1).Position());
    this->joint = dynamic_cast<rp::FixedJoint*>(workspace().value()->physicsWorld->createJoint(jointInfo));
}