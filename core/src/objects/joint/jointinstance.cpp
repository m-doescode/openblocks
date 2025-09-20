#include "jointinstance.h"

#include "datatypes/ref.h"
#include "objects/base/instance.h"
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

void JointInstance::OnPhysicsStep(float deltaTime) {
}

template <typename T>
bool weak_equals(std::weak_ptr<T> a, std::weak_ptr<T> b) {
    return (a.expired() && b.expired()) || (!a.expired() && !b.expired() && a.lock() == b.lock());
}

void JointInstance::onUpdated(std::string property) {
    if (property == "Part0" || property == "Part1")
        Update();
}

void JointInstance::Update() {
    // Truly checking to see if the joint has been modified, etc. is FAR too complex for what its worth, so I'll just stick to break-detach-build-attach
    breakJoint();

    if (!oldPart0.expired())
        oldPart0.lock()->RemoveJoint(shared<JointInstance>());
    if (!oldPart1.expired())
        oldPart1.lock()->RemoveJoint(shared<JointInstance>());

    oldPart0 = part0;
    oldPart1 = part1;

    bool isValid = GetParent() != nullptr && (GetParent()->GetClass() == &JointsService::TYPE
        ? CheckInstanceWorkspaceValidity({part0, part1}) : CheckInstanceWorkspaceValidity({part0, part1, shared_from_this()}));
    if (!isValid)
        return;

    buildJoint();

    part0.lock()->AddJoint(shared<JointInstance>());
    part1.lock()->AddJoint(shared<JointInstance>());
}

bool JointInstance::isDrivenJoint() {
    return false;
}

nullable std::shared_ptr<Workspace> JointInstance::workspaceOfPart(std::shared_ptr<BasePart> part) {
    return part->workspace();
}