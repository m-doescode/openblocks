#include <cstdio>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <reactphysics3d/collision/RaycastInfo.h>
#include <reactphysics3d/collision/shapes/BoxShape.h>
#include <reactphysics3d/collision/shapes/CollisionShape.h>
#include <reactphysics3d/components/RigidBodyComponents.h>
#include <reactphysics3d/engine/EventListener.h>
#include <reactphysics3d/engine/PhysicsCommon.h>
#include <reactphysics3d/mathematics/Quaternion.h>
#include <reactphysics3d/mathematics/Ray.h>
#include <reactphysics3d/mathematics/Transform.h>
#include <reactphysics3d/mathematics/Vector3.h>
#include <reactphysics3d/memory/DefaultAllocator.h>
#include <reactphysics3d/memory/MemoryAllocator.h>
#include <reactphysics3d/reactphysics3d.h>
#include "../common.h"
#include "../objects/part.h"
#include "datatypes/cframe.h"
#include "util.h"

#include "simulation.h"

namespace rp = reactphysics3d;

class PhysicsListener : public rp::EventListener {
    void onContact(const CollisionCallback::CallbackData& /*callbackData*/) override {
        // printf("Collision occurred!\n");
    }
};

rp::PhysicsCommon* physicsCommon;
rp::PhysicsWorld* world;
PhysicsListener eventListener;

void simulationInit() {
    physicsCommon = new rp::PhysicsCommon; // I allocate this on the heap to ensure it exists while Parts are getting destructed. This is probably not great
    world = physicsCommon->createPhysicsWorld();

    world->setGravity(rp::Vector3(0, -196.2, 0));

    world->setEventListener(&eventListener);
}

void syncPartPhysics(std::shared_ptr<Part> part) {
    glm::mat4 rotMat = glm::mat4(1.0f);

    rp::Transform transform = part->cframe;
    if (!part->rigidBody) {
        part->rigidBody = world->createRigidBody(transform);
    } else {
        part->rigidBody->setTransform(transform);
    }

    rp::BoxShape* shape = physicsCommon->createBoxShape(glmToRp(part->scale * glm::vec3(0.5f)));

    if (part->rigidBody->getNbColliders() > 0) {
        part->rigidBody->removeCollider(part->rigidBody->getCollider(0));
    }

    if (part->rigidBody->getNbColliders() == 0)
        part->rigidBody->addCollider(shape, rp::Transform());
    part->rigidBody->setType(part->anchored ? rp::BodyType::STATIC : rp::BodyType::DYNAMIC);
    part->rigidBody->getCollider(0)->setCollisionCategoryBits(0b11);

    part->rigidBody->setUserData(&*part);
}

void physicsStep(float deltaTime) {
    // Step the simulation a few steps
    world->update(deltaTime / 2);

    // Naive implementation. Parts are only considered so if they are just under Workspace
    // TODO: Add list of tracked parts in workspace based on their ancestry using inWorkspace property of Instance
    for (InstanceRef obj : workspace()->GetChildren()) {
        if (obj->GetClass()->className != "Part") continue; // TODO: Replace this with a .IsA call instead of comparing the class name directly
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(obj);
        const rp::Transform& transform = part->rigidBody->getTransform();
        part->cframe = Data::CFrame(transform);
    }
}

RaycastResult::RaycastResult(const rp::RaycastInfo& raycastInfo)
    : worldPoint(raycastInfo.worldPoint)
    , worldNormal(raycastInfo.worldNormal)
    , hitFraction(raycastInfo.hitFraction)
    , triangleIndex(raycastInfo.triangleIndex)
    , body(raycastInfo.body)
    , collider(raycastInfo.collider) {}

class NearestRayHit : public rp::RaycastCallback {
    rp::Vector3 startPos;
    std::optional<RaycastFilter> filter;

    std::optional<RaycastResult> nearestHit;
    float nearestHitDistance = -1;

    // Order is not guaranteed, so we have to figure out the nearest object using a more sophisticated algorith,
    rp::decimal notifyRaycastHit(const rp::RaycastInfo& raycastInfo) override {
        // If the detected object is further away than the nearest object, continue.
        int distance = (raycastInfo.worldPoint - startPos).length();
        if (nearestHitDistance != -1 && distance >= nearestHitDistance)
            return 1;

        if (!filter) {
            nearestHit = raycastInfo;
            nearestHitDistance = distance;
            return 1;
        }

        std::shared_ptr<Part> part = partFromBody(raycastInfo.body);
        FilterResult result = filter.value()(part);
        if (result == FilterResult::BLOCK) {
            nearestHit = std::nullopt;
            nearestHitDistance = distance;
            return 1;
        } else if (result == FilterResult::TARGET) {
            nearestHit = raycastInfo;
            nearestHitDistance = distance;
            return 1;
        }
        return 1;
    };

public:
    NearestRayHit(rp::Vector3 startPos, std::optional<RaycastFilter> filter = std::nullopt) : startPos(startPos), filter(filter) {}
    std::optional<const RaycastResult> getNearestHit() { return nearestHit; };
};

std::optional<const RaycastResult> castRayNearest(glm::vec3 point, glm::vec3 rotation, float maxLength, std::optional<RaycastFilter> filter, unsigned short categoryMaskBits) {
    rp::Ray ray(glmToRp(point), glmToRp(glm::normalize(rotation)) * maxLength);
    NearestRayHit rayHit(glmToRp(point), filter);
    world->raycast(ray, &rayHit, categoryMaskBits);
    return rayHit.getNearestHit();
}