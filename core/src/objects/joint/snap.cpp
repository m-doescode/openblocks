#include "snap.h"

#include "datatypes/cframe.h"
#include "objects/datamodel.h"
#include "objects/joint/jointinstance.h"
#include "objects/service/jointsservice.h"
#include "objects/part/part.h"
#include "objects/service/workspace.h"
#include "physics/world.h"
#include <memory>

Snap::Snap(): JointInstance(&TYPE) {
}

Snap::~Snap() {
}

void Snap::buildJoint() {
    std::shared_ptr<Workspace> workspace = workspaceOfPart(part0.lock());

    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    CFrame newFrame = part0.lock()->cframe * (c0 * c1.Inverse());
    part1.lock()->cframe = newFrame;

    PhysFixedJointInfo jointInfo(c0, c1);
    this->joint = workspace->CreateJoint(jointInfo, part0.lock(), part1.lock());
    jointWorkspace = workspace;
}

void Snap::breakJoint() {
    if (joint.jointImpl != nullptr) {
        joint.parentWorld->destroyJoint(joint);
        joint = {};
    }
}