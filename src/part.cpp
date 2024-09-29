#include <glm/ext.hpp>
#include <reactphysics3d/engine/PhysicsCommon.h>
#include <reactphysics3d/engine/PhysicsWorld.h>
#include "physics/util.h"

#include "part.h"

namespace rp = reactphysics3d;

extern rp::PhysicsWorld* world;
extern rp::PhysicsCommon physicsCommon;

void Part::syncTransforms() {
    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat = glm::rotate(rotMat, rotation.x, glm::vec3(1., 0., 0.));
    rotMat = glm::rotate(rotMat, rotation.y, glm::vec3(0., 1., 0.));
    rotMat = glm::rotate(rotMat, rotation.z, glm::vec3(0., 0., 1.));
    glm::quat quat(rotMat);

    rp::Transform transform(glmToRp(position), glmToRp(quat));
    rigidBody = world->createRigidBody(transform);

    rp::BoxShape* shape = physicsCommon.createBoxShape(glmToRp(scale * glm::vec3(0.5)));

    rigidBody->removeCollider(rigidBody->getCollider(0));
    rigidBody->addCollider(shape, rp::Transform());
}