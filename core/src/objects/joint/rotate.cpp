#include "rotate.h"
#include "objects/part/basepart.h" // IWYU pragma: keep
#include "objects/service/workspace.h"
#include "rendering/renderer.h"

INSTANCE_IMPL(Rotate)

InstanceType Rotate::__buildType() {
    return make_instance_type<Rotate, JointInstance>("Rotate");
}

Rotate::~Rotate() {
}

static CFrame XYZToZXY(glm::vec3(0, 0, 0), -glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

void Rotate::buildJoint() {
    std::shared_ptr<Workspace> workspace = workspaceOfPart(part0.lock());

    // Update Part1's rotation and cframe prior to creating the joint as reactphysics3d locks rotation based on how it
    // used to be rather than specifying an anchor rotation, so whatever.
    CFrame newFrame = part0.lock()->cframe * (c0 * c1.Inverse());
    part1.lock()->cframe = newFrame;

    // Do NOT use Abs() in this scenario. For some reason that breaks it
    PhysRotatingJointInfo jointInfo(c0, c1);
    this->joint = workspace->CreateJoint(jointInfo, part0.lock(), part1.lock());
    jointWorkspace = workspace;
}