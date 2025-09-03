#include "motor6d.h"
#include "objects/part/part.h"
#include "objects/service/workspace.h"
#include "rendering/renderer.h"

#define PI 3.14159

Motor6D::Motor6D(): JointInstance(&TYPE) {
}

Motor6D::~Motor6D() {
}
static CFrame XYZToZXY(glm::vec3(0, 0, 0), -glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

void Motor6D::buildJoint() {
    std::shared_ptr<Workspace> workspace = workspaceOfPart(part0.lock());

    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    CFrame newFrame = part0.lock()->cframe * (c0 * c1.Inverse());
    part1.lock()->cframe = newFrame;
    PhysStepperJointInfo jointInfo(c0, c1, desiredAngle, maxVelocity);
    
    this->joint = workspace->CreateJoint(jointInfo, part0.lock(), part1.lock());
    jointWorkspace = workspace;
}

void Motor6D::onUpdated(std::string property) {
    if (property == "DesiredAngle") {
        joint.setTargetAngle(desiredAngle);
    } else if (property == "MaxVelocity") {
        joint.setAngularVelocity(maxVelocity);
    }
}