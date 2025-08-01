#include "handles.h"
#include "common.h"
#include "datatypes/cframe.h"
#include "datatypes/vector.h"
#include "math_helper.h"
#include "objects/service/selection.h"
#include "partassembly.h"
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

std::shared_ptr<BasePart> getHandleAdornee() {
    std::shared_ptr<Selection> selection = gDataModel->GetService<Selection>();
    for (std::weak_ptr<Instance> inst : selection->Get()) {
        if (!inst.expired() && inst.lock()->IsA<BasePart>())
            return inst.lock()->CastTo<BasePart>().expect();
    }

    return {};
}

CFrame partCFrameFromHandlePos(HandleFace face, Vector3 newPos) {
    auto adornee = getHandleAdornee();
    if (adornee == nullptr) return CFrame(glm::vec3(0,0,0), (Vector3)glm::vec3(0,0,0));

    CFrame localFrame = editorToolHandles.worldMode ? CFrame::IDENTITY + adornee->position() : adornee->cframe;
    CFrame inverseFrame = localFrame.Inverse();
    Vector3 handleOffset = editorToolHandles.worldMode ? ((Vector3::ONE * 2.f) + adornee->GetAABB() * 0.5f) : Vector3(2.f) + adornee->size * 0.5f;

    Vector3 handlePos = localFrame * (handleOffset * face.normal);

    // glm::vec3 localPos = inverseFrame * newPos;
    glm::vec3 newPartPos = newPos - localFrame.Rotation() * (handleOffset * face.normal);
    return adornee->cframe.Rotation() + newPartPos;
}

std::optional<HandleFace> raycastHandle(rp3d::Ray ray) {
    for (HandleFace face : HandleFace::Faces) {
        CFrame cframe = getHandleCFrame(face);
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

static int getAABBOfSelection(glm::vec3& pos, glm::vec3& size, glm::vec3& min, glm::vec3& max) {
    int count = 0;
    std::shared_ptr<Selection> selection = gDataModel->GetService<Selection>();
    for (std::weak_ptr<Instance> inst : selection->Get()) {
        if (inst.expired() || !inst.lock()->IsA<BasePart>()) continue;
        std::shared_ptr<BasePart> part = inst.lock()->CastTo<BasePart>().expect();

        if (count == 0)
            min = part->position(), max = part->position();
        count++;

        Vector3 aabbSize = part->GetAABB();
        expandAABB(min, max, part->position() - aabbSize / 2.f);
        expandAABB(min, max, part->position() + aabbSize / 2.f);
    }

    getAABBCoords(pos, size, min, max);

    return count;
}

static std::shared_ptr<BasePart> getFirstSelectedPart() {
    std::shared_ptr<Selection> selection = gDataModel->GetService<Selection>();
    for (std::weak_ptr<Instance> inst : selection->Get()) {
        if (inst.expired() || !inst.lock()->IsA<BasePart>()) continue;
        
        return inst.lock()->CastTo<BasePart>().expect();
    }

    return {};
}

CFrame getLocalHandleCFrame(HandleFace face) {
    PartAssembly assembly = PartAssembly::FromSelection(gDataModel->GetService<Selection>());

    Vector3 size;
    if (editorToolHandles.worldMode)
        size = assembly.bounds();
    else
        size = assembly.size();

    // Since rotation displays rings, all handles must be the same distance from origin in order for the
    // rings to be circular
    if (editorToolHandles.handlesType == HandlesType::RotateHandles)
        size = Vector3::ONE * fmax(fmax(size.X(), size.Y()), size.Z());

    CFrame cframe = CFrame::pointToward(face.normal * ((glm::vec3)size * 0.5f + 2.f), -face.normal);
    return cframe;
}

CFrame getHandleCFrame(HandleFace face) {
    PartAssembly assembly = PartAssembly::FromSelection(gDataModel->GetService<Selection>());

    if (editorToolHandles.worldMode) {
        return getLocalHandleCFrame(face) + assembly.assemblyOrigin().Position();
    } else
        return assembly.assemblyOrigin() * getLocalHandleCFrame(face);
}