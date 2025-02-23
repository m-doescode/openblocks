#include "handles.h"
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

    // return adornee->lock()->cframe + face.normal * 5.f;
    if (worldMode)
        return adornee->lock()->cframe + (adornee->lock()->size * 0.5f * face.normal) + face.normal * 5.f;
    return adornee->lock()->cframe + glm::vec3(glm::mat4(adornee->lock()->cframe.Rotation()) * glm::vec4((adornee->lock()->size * 0.5f * face.normal) + face.normal * 5.f, 0));
}

Data::CFrame Handles::PartCFrameFromHandlePos(HandleFace face, Data::Vector3 newPos) {
    if (!adornee.has_value() || adornee->expired()) return Data::CFrame(glm::vec3(0,0,0), (Data::Vector3)glm::vec3(0,0,0));

    // return adornee->lock()->cframe + face.normal * 5.f;
    if (worldMode)
        return adornee->lock()->cframe.Rotation() + newPos - (adornee->lock()->size * 0.5f * face.normal) - face.normal * 5.f;
    return adornee->lock()->cframe.Rotation() + newPos - glm::vec3(glm::mat4(adornee->lock()->cframe.Rotation()) * glm::vec4((adornee->lock()->size * 0.5f * face.normal) + face.normal * 5.f, 0));
}

std::optional<HandleFace> Handles::RaycastHandle(rp3d::Ray ray) {
    for (HandleFace face : HandleFace::Faces) {
        Data::CFrame cframe = GetCFrameOfHandle(face);
        // Implement manual detection via boxes instead of... this shit
        rp3d::RigidBody* body = world->createRigidBody(cframe);
        body->addCollider(common.createBoxShape(Data::Vector3(.5, .5, .5)), rp3d::Transform::identity());

        rp3d::RaycastInfo info;
        if (body->raycast(ray, info)) {
            world->destroyRigidBody(body);
            return face;
        }

        world->destroyRigidBody(body);
    }

    return std::nullopt;
}