#include "handles.h"
#include "common.h"
#include "datatypes/cframe.h"
#include "datatypes/vector.h"
#include <optional>
#include <reactphysics3d/collision/RaycastInfo.h>
#include <reactphysics3d/engine/PhysicsCommon.h>
#include <reactphysics3d/engine/PhysicsWorld.h>
#include <reactphysics3d/mathematics/Transform.h>

HandleFace HandleFace::XPos(0, glm::vec3(1,0,0));
HandleFace HandleFace::XNeg(1, glm::vec3(-1,0,0));
HandleFace HandleFace::YPos(2, glm::vec3(0,1,0));
HandleFace HandleFace::YNeg(3, glm::vec3(0,-1,0));
HandleFace HandleFace::ZPos(4, glm::vec3(0,0,1));
HandleFace HandleFace::ZNeg(5, glm::vec3(0,0,-1));
std::array<HandleFace, 6> HandleFace::Faces { HandleFace::XPos, HandleFace::XNeg, HandleFace::YPos, HandleFace::YNeg, HandleFace::ZPos, HandleFace::ZNeg };

// Shitty solution
static rp3d::PhysicsCommon common;
static rp3d::PhysicsWorld* world = common.createPhysicsWorld();

const InstanceType Handles::TYPE = {
    .super = &Instance::TYPE,
    .className = "Handles",
    // .constructor = &Workspace::Create,
    // .explorerIcon = "",
};

const InstanceType* Handles::GetClass() {
    return &TYPE;
}

Handles::Handles(): Instance(&TYPE) {
}

Data::CFrame Handles::GetCFrameOfHandle(HandleFace face) {
    if (!adornee.has_value() || adornee->expired()) return Data::CFrame(glm::vec3(0,0,0), (Data::Vector3)glm::vec3(0,0,0));

    Data::CFrame localFrame = worldMode ? Data::CFrame::IDENTITY + adornee->lock()->position() : adornee->lock()->cframe;

    // We don't want this to align with local * face.normal, or else we have problems.
    glm::vec3 upAxis(0, 0, 1);
    if (glm::abs(glm::dot(glm::vec3(localFrame.Rotation() * face.normal), upAxis)) > 0.9999f)
        upAxis = glm::vec3(0, 1, 0);

    Data::Vector3 handleOffset = this->worldMode ? ((Data::Vector3::ONE * 2.f) + adornee->lock()->GetAABB() * 0.5f) : Data::Vector3(2.f + adornee->lock()->size * 0.5f);
    Data::Vector3 handlePos = localFrame * (handleOffset * face.normal);
    Data::CFrame cframe(handlePos, handlePos + localFrame.Rotation() * face.normal, upAxis);

    return cframe;
}

Data::CFrame Handles::PartCFrameFromHandlePos(HandleFace face, Data::Vector3 newPos) {
    if (!adornee.has_value() || adornee->expired()) return Data::CFrame(glm::vec3(0,0,0), (Data::Vector3)glm::vec3(0,0,0));

    Data::CFrame localFrame = worldMode ? Data::CFrame::IDENTITY + adornee->lock()->position() : adornee->lock()->cframe;
    Data::CFrame inverseFrame = localFrame.Inverse();
    Data::Vector3 handleOffset = this->worldMode ? ((Data::Vector3::ONE * 2.f) + adornee->lock()->GetAABB() * 0.5f) : Data::Vector3(2.f + adornee->lock()->size * 0.5f);

    Data::Vector3 handlePos = localFrame * (handleOffset * face.normal);

    // glm::vec3 localPos = inverseFrame * newPos;
    glm::vec3 newPartPos = newPos - localFrame.Rotation() * (handleOffset * face.normal);
    return adornee->lock()->cframe.Rotation() + newPartPos;
}

std::optional<HandleFace> Handles::RaycastHandle(rp3d::Ray ray) {
    for (HandleFace face : HandleFace::Faces) {
        Data::CFrame cframe = GetCFrameOfHandle(face);
        // Implement manual detection via boxes instead of... this shit
        // This code also hardly works, and is not good at all... Hooo nope.
        rp3d::RigidBody* body = world->createRigidBody(Data::CFrame::IDENTITY + cframe.Position());
        body->addCollider(common.createBoxShape(cframe.Rotation() * Data::Vector3(HandleSize(face) / 2.f)), rp3d::Transform::identity());

        rp3d::RaycastInfo info;
        if (body->raycast(ray, info)) {
            world->destroyRigidBody(body);
            return face;
        }

        world->destroyRigidBody(body);
    }

    return std::nullopt;
}

Data::Vector3 Handles::HandleSize(HandleFace face) {
    if (handlesType == HandlesType::MoveHandles)
        return glm::vec3(0.5f, 0.5f, 2.f);
    return glm::vec3(1,1,1);
}