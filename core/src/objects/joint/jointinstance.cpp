#include "jointinstance.h"

#include "datatypes/cframe.h"
#include "datatypes/ref.h"
#include "objects/datamodel.h"
#include "objects/jointsservice.h"
#include "objects/part.h"
#include "objects/workspace.h"
#include <memory>
#include <reactphysics3d/constraint/FixedJoint.h>
#include <reactphysics3d/engine/PhysicsWorld.h>
#include "ptr_helpers.h"

const InstanceType JointInstance::TYPE = {
    .super = &Instance::TYPE,
    .className = "JointInstance",
};

const InstanceType* JointInstance::GetClass() {
    return &TYPE;
}

JointInstance::JointInstance(const InstanceType* type): Instance(type) {
    this->memberMap = std::make_unique<MemberMap>(MemberMap {
        .super = std::move(this->memberMap),
        .members = {
            { "Part0", {
                .backingField = &part0,
                .type = &Data::InstanceRef::TYPE,
                .codec = fieldCodecOf<Data::InstanceRef, std::weak_ptr<Instance>>(),
                .updateCallback = memberFunctionOf(&JointInstance::onUpdated, this),
            }}, { "Part1", {
                .backingField = &part1,
                .type = &Data::InstanceRef::TYPE,
                .codec = fieldCodecOf<Data::InstanceRef, std::weak_ptr<Instance>>(),
                .updateCallback = memberFunctionOf(&JointInstance::onUpdated, this),
            }}, { "C0", {
                .backingField = &c0,
                .type = &Data::CFrame::TYPE,
                .codec = fieldCodecOf<Data::CFrame>(),
                .updateCallback = memberFunctionOf(&JointInstance::onUpdated, this),
            }}, { "C1", {
                .backingField = &c1,
                .type = &Data::CFrame::TYPE,
                .codec = fieldCodecOf<Data::CFrame>(),
                .updateCallback = memberFunctionOf(&JointInstance::onUpdated, this),
            }}, 
        }
    });
}

JointInstance::~JointInstance() {
}

void JointInstance::OnAncestryChanged(std::optional<std::shared_ptr<Instance>>, std::optional<std::shared_ptr<Instance>>) {
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

std::optional<std::shared_ptr<Workspace>> JointInstance::workspaceOfPart(std::shared_ptr<Part> part) {
    return part->workspace();
}