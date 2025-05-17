#include "handles.h"
#include "common.h"
#include "datatypes/cframe.h"
#include "datatypes/vector.h"
#include <glm/ext/scalar_common.hpp>
#include <memory>
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

static CFrame XYZToZXY(glm::vec3(0, 0, 0), -glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

// Shitty solution
static rp3d::PhysicsCommon common;
static rp3d::PhysicsWorld* world = common.createPhysicsWorld();

std::shared_ptr<Part> getHandleAdornee() {
    for (std::weak_ptr<Instance> inst : getSelection()) {
        if (!inst.expired() && inst.lock()->IsA<Part>())
            return inst.lock()->CastTo<Part>().expect();
    }

    return {};
}

CFrame getCFrameOfHandle(HandleFace face) {
    auto adornee = getHandleAdornee();
    if (adornee == nullptr) return CFrame(glm::vec3(0,0,0), (Vector3)glm::vec3(0,0,0));

    CFrame localFrame = editorToolHandles.worldMode ? CFrame::IDENTITY + adornee->position() : adornee->cframe;

    Vector3 handleNormal = face.normal;
    if (editorToolHandles.nixAxes)
        handleNormal = XYZToZXY * face.normal;

    // We don't want this to align with local * face.normal, or else we have problems.
    glm::vec3 upAxis(0, 0, 1);
    if (glm::abs(glm::dot(glm::vec3(localFrame.Rotation() * handleNormal), upAxis)) > 0.9999f)
        upAxis = glm::vec3(0, 1, 0);

    glm::vec3 partSize = editorToolHandles.handlesType == HandlesType::RotateHandles ? glm::vec3(glm::max(adornee->size.x, adornee->size.y, adornee->size.z)) : adornee->size;
    Vector3 handleOffset = editorToolHandles.worldMode ? ((Vector3::ONE * 2.f) + adornee->GetAABB() * 0.5f) : Vector3(2.f + partSize * 0.5f);
    Vector3 handlePos = localFrame * (handleOffset * handleNormal);
    CFrame cframe(handlePos, handlePos + localFrame.Rotation() * -handleNormal, upAxis);

    return cframe;
}

CFrame partCFrameFromHandlePos(HandleFace face, Vector3 newPos) {
    auto adornee = getHandleAdornee();
    if (adornee == nullptr) return CFrame(glm::vec3(0,0,0), (Vector3)glm::vec3(0,0,0));

    CFrame localFrame = editorToolHandles.worldMode ? CFrame::IDENTITY + adornee->position() : adornee->cframe;
    CFrame inverseFrame = localFrame.Inverse();
    Vector3 handleOffset = editorToolHandles.worldMode ? ((Vector3::ONE * 2.f) + adornee->GetAABB() * 0.5f) : Vector3(2.f + adornee->size * 0.5f);

    Vector3 handlePos = localFrame * (handleOffset * face.normal);

    // glm::vec3 localPos = inverseFrame * newPos;
    glm::vec3 newPartPos = newPos - localFrame.Rotation() * (handleOffset * face.normal);
    return adornee->cframe.Rotation() + newPartPos;
}

std::optional<HandleFace> raycastHandle(rp3d::Ray ray) {
    for (HandleFace face : HandleFace::Faces) {
        CFrame cframe = getCFrameOfHandle(face);
        // Implement manual detection via boxes instead of... this shit
        // This code also hardly works, and is not good at all... Hooo nope.
        rp3d::RigidBody* body = world->createRigidBody(CFrame::IDENTITY + cframe.Position());
        body->addCollider(common.createBoxShape(cframe.Rotation() * Vector3(handleSize(face) / 2.f)), rp3d::Transform::identity());

        rp3d::RaycastInfo info;
        if (body->raycast(ray, info)) {
            world->destroyRigidBody(body);
            return face;
        }

        world->destroyRigidBody(body);
    }

    return std::nullopt;
}

Vector3 handleSize(HandleFace face) {
    if (editorToolHandles.handlesType == HandlesType::MoveHandles)
        return glm::vec3(0.5f, 0.5f, 2.f);
    return glm::vec3(1,1,1);
}