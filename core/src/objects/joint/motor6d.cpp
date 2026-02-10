#include "motor6d.h"
#include "datatypes/vector.h"
#include "objectmodel/property.h"
#include "objects/part/part.h"
#include "objects/service/workspace.h"
#include "rendering/renderer.h"

#define PI 3.14159

InstanceType Motor6D::__buildType() {
    return make_instance_type<Motor6D, JointInstance>(
        "Motor6D",

        def_property("currentAngle", &Motor6D::currentAngle),
        def_property("desiredAngle", &Motor6D::desiredAngle),
        def_property("maxVelocity", &Motor6D::maxVelocity)
    );
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

bool Motor6D::isDrivenJoint() {
    return true;
}

void Motor6D::OnPhysicsStep(float deltaTime) {
    // Tween currentAngle
    float diffAngle = abs(currentAngle - desiredAngle);
    if (diffAngle > abs(maxVelocity)) { // Don't tween if we're already close enough to being there
        if (currentAngle < desiredAngle)
            currentAngle += maxVelocity;
        else
            currentAngle -= maxVelocity;
    }

    // Shouldn't in theory be necessary, but just in case.
    if (part0.expired() || part1.expired()) return;

    // TODO: Re-add rotating only one part when both are unanchored, maybe?
    joint.setTargetAngle(currentAngle);
}