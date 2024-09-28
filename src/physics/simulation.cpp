#include <reactphysics3d/reactphysics3d.h>
#include <vector>
#include "../part.h"

#include "simulation.h"

namespace rp = reactphysics3d;

extern std::vector<Part> parts;

rp::PhysicsCommon physicsCommon;
rp::PhysicsWorld* world;
rp::RigidBody* body;

void simulationInit() {
    world = physicsCommon.createPhysicsWorld();

    rp::Vector3 position(0, 20, 0);
    rp::Quaternion orientation = rp::Quaternion::identity();
    rp::Transform transform(position, orientation);
    body = world->createRigidBody(transform);
}

void physicsStep(float deltaTime) {
    // Step the simulation a few steps
    world->update(deltaTime);

    // Get the updated position of the body
    const rp::Transform& transform = body->getTransform();
    const rp::Vector3& position = transform.getPosition();

    // Display the position of the body
    std::cout << "Body Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}