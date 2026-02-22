#include "rotatev.h"
#include "objects/part/basepart.h" // IWYU pragma: keep
#include "objects/service/workspace.h"
#include "rendering/renderer.h"

INSTANCE_IMPL(RotateV)

#define PI 3.14159

InstanceType RotateV::__buildType() {
    return make_instance_type<RotateV, JointInstance>("RotateV");
}

RotateV::~RotateV() {
}

static CFrame XYZToZXY(glm::vec3(0, 0, 0), -glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

void RotateV::buildJoint() {
    std::shared_ptr<Workspace> workspace = workspaceOfPart(part0.lock());

    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    CFrame newFrame = part0.lock()->cframe * (c0 * c1.Inverse());
    part1.lock()->cframe = newFrame;
    // Do NOT use Abs() in this scenario. For some reason that breaks it
    float vel = part0.lock()->GetSurfaceParamB(-c0.LookVector().Unit()) * 6.28;
    PhysMotorizedJointInfo jointInfo(c0, c1, vel);
    
    this->joint = workspace->CreateJoint(jointInfo, part0.lock(), part1.lock());
    jointWorkspace = workspace;
}

void RotateV::OnPartParamsUpdated() {
    float vel = part0.lock()->GetSurfaceParamB(-c0.LookVector().Unit()) * 6.28;
    this->joint.setAngularVelocity(vel);
}