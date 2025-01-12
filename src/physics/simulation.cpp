#include <cstdio>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <reactphysics3d/collision/shapes/BoxShape.h>
#include <reactphysics3d/collision/shapes/CollisionShape.h>
#include <reactphysics3d/components/RigidBodyComponents.h>
#include <reactphysics3d/mathematics/Quaternion.h>
#include <reactphysics3d/mathematics/Transform.h>
#include <reactphysics3d/mathematics/Vector3.h>
#include <reactphysics3d/memory/DefaultAllocator.h>
#include <reactphysics3d/memory/MemoryAllocator.h>
#include <reactphysics3d/reactphysics3d.h>
#include <vector>
#include "../part.h"
#include "util.h"

#include "simulation.h"

namespace rp = reactphysics3d;

extern std::vector<Part> parts;

rp::PhysicsCommon physicsCommon;
rp::PhysicsWorld* world;

void simulationInit() {
    world = physicsCommon.createPhysicsWorld();

    world->setGravity(rp::Vector3(0, -196.2, 0));
}

void syncPartPhysics(Part& part) {
    glm::mat4 rotMat = glm::mat4(1.0f);

    rp::Transform transform(glmToRp(part.position), glmToRp(part.rotation));
    if (!part.rigidBody) {
        part.rigidBody = world->createRigidBody(transform);
    } else {
        part.rigidBody->setTransform(transform);
    }

    rp::BoxShape* shape = physicsCommon.createBoxShape(glmToRp(part.scale * glm::vec3(0.5f)));

    if (part.rigidBody->getNbColliders() > 0) {
        part.rigidBody->removeCollider(part.rigidBody->getCollider(0));
    }

    if (part.rigidBody->getNbColliders() == 0)
    part.rigidBody->addCollider(shape, rp::Transform());
    part.rigidBody->setType(part.anchored ? rp::BodyType::STATIC : rp::BodyType::DYNAMIC);
}

void physicsStep(float deltaTime) {
    // Step the simulation a few steps
    world->update(deltaTime / 2);

    for (Part& part : parts) {
        const rp::Transform& transform = part.rigidBody->getTransform();
        part.position = rpToGlm(transform.getPosition());
        // part.rotation = glm::eulerAngles(rpToGlm(transform.getOrientation()));
        part.rotation = rpToGlm(transform.getOrientation());
    }
}