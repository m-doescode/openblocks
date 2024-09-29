#include <cstdio>
#include <glm/ext/matrix_float3x3.hpp>
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

    rp::Vector3 position(0, 20, 0);
    rp::Quaternion orientation = rp::Quaternion::identity();
    rp::Transform transform(position, orientation);
    // body = world->createRigidBody(transform);
}

void addToSimulation(Part part) {
    if (part.rigidBody) {
        fprintf(stderr, "Attempt to add part to simulation that already has a rigid body.\n");
        return;
    }

    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat = glm::rotate(rotMat, part.rotation.x, glm::vec3(1., 0., 0.));
    rotMat = glm::rotate(rotMat, part.rotation.y, glm::vec3(0., 1., 0.));
    rotMat = glm::rotate(rotMat, part.rotation.z, glm::vec3(0., 0., 1.));
    glm::quat quat(rotMat);

    rp::Transform transform(glmToRp(part.position), glmToRp(quat));
    part.rigidBody = world->createRigidBody(transform);

    rp::BoxShape* shape = physicsCommon.createBoxShape(rp::Vector3(.5, .5, .5));

    part.rigidBody->addCollider(shape, rp::Transform());
    part.rigidBody->setType(part.anchored ? rp::BodyType::STATIC : rp::BodyType::DYNAMIC);
}

void physicsStep(float deltaTime) {
    // Step the simulation a few steps
    world->update(deltaTime);

    // Get the updated position of the body
    // const rp::Transform& transform = body->getTransform();
    // const rp::Vector3& position = transform.getPosition();

    // Display the position of the body
    // std::cout << "Body Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}