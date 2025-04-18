#include "snap.h"

#include "datatypes/cframe.h"
#include "datatypes/ref.h"
#include "objects/datamodel.h"
#include "objects/jointsservice.h"
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

void Snap::OnAncestryChanged(std::optional<std::shared_ptr<Instance>>, std::optional<std::shared_ptr<Instance>>) {
    // If the old workspace existed, and the new one differs, delete the current joint
    if (this->joint && !this->oldWorkspace.expired() && (!workspace() || workspace().value() != this->oldWorkspace.lock())) {
        // printf("Broke joint - Removed from workspace\n");
        oldJointWorkspace.lock()->physicsWorld->destroyJoint(this->joint);
        this->joint = nullptr;
    }

    // If the previous parent was JointsService, and now it isn't, delete the joint
    if (this->joint && !oldParent.expired() && oldParent.lock()->GetClass() == &JointsService::TYPE && (!GetParent() || GetParent() != oldParent.lock())) {
        // printf("Broke joint - Removed from JointsService\n");
        oldJointWorkspace.lock()->physicsWorld->destroyJoint(this->joint);
        this->joint = nullptr;
    }

    // If the new workspace exists, and the old one differs, create the joint
    if (!this->joint && workspace() && (oldWorkspace.expired() || oldWorkspace.lock() != workspace().value())) {
        // printf("Made joint - Added to workspace\n");
        buildJoint();
    }

    // If the new parent is JointsService and the previous wasn't, then create the joint
    if (!this->joint && GetParent() && GetParent().value()->GetClass() == &JointsService::TYPE && (oldParent.expired() || GetParent() != oldParent.lock())) {
        // printf("Made joint - Added to JointsService\n");
        buildJoint();
    }

    this->oldParent = !GetParent() ? std::weak_ptr<Instance>() : GetParent().value();
    this->oldWorkspace = !workspace() ? std::weak_ptr<Workspace>() : workspace().value();
    this->oldJointWorkspace = !jointWorkspace() ? std::weak_ptr<Workspace>() : jointWorkspace().value();
}

void Snap::onUpdated(std::string property) {
    // We are not in the workspace, so we don't really care what values are currently set
    if (!jointWorkspace()) return;

    // Workspace cannot have changed, so if the joint currently exists, it is in the present one
    if (this->joint)
        jointWorkspace().value()->physicsWorld->destroyJoint(this->joint);

    buildJoint();
}

void Snap::buildJoint() {
    if (part0.expired() || part1.expired() || !jointWorkspace()) return;

    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    Data::CFrame newFrame = part0.lock()->cframe * (c1.Inverse() * c0);
    part1.lock()->cframe = newFrame;
    jointWorkspace().value()->SyncPartPhysics(part1.lock());

    rp::FixedJointInfo jointInfo(part0.lock()->rigidBody, part1.lock()->rigidBody, (c0.Inverse() * c1).Position());
    this->joint = dynamic_cast<rp::FixedJoint*>(jointWorkspace().value()->physicsWorld->createJoint(jointInfo));
}

std::optional<std::shared_ptr<Workspace>> Snap::jointWorkspace() {
    if (workspace()) return workspace();

    if (GetParent() && GetParent().value()->GetClass() == &JointsService::TYPE)
        return std::dynamic_pointer_cast<DataModel>(GetParent().value()->GetParent().value())->GetService<Workspace>("Workspace");

    return {};
}