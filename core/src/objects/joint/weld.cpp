#include "weld.h"

#include "datatypes/cframe.h"
#include "objects/datamodel.h"
#include "objects/joint/jointinstance.h"
#include "objects/service/jointsservice.h"
#include "objects/part/part.h"
#include "objects/service/workspace.h"
#include "physics/world.h"
#include <memory>
#include <reactphysics3d/constraint/FixedJoint.h>
#include <reactphysics3d/engine/PhysicsWorld.h>

Weld::Weld(): JointInstance(&TYPE) {
}

Weld::~Weld() {
}

void Weld::buildJoint() {
    std::shared_ptr<Workspace> workspace = workspaceOfPart(part0.lock());

    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    CFrame newFrame = part0.lock()->cframe * (c0 * c1.Inverse());
    part1.lock()->cframe = newFrame;
    part1.lock()->UpdateProperty("CFrame");

    PhysJointWeldInfo jointInfo((c0.Inverse() * c1).Position());
    this->joint = workspace->CreateJoint(jointInfo, part0.lock(), part1.lock());
    jointWorkspace = workspace;
}