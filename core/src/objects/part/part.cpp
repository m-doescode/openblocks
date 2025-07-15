#include "part.h"
#include "enum/part.h"
#include "physics/util.h"
#include <glm/common.hpp>

Part::Part(): BasePart(&TYPE) {
    _lastShape = shape;
    _lastSize = size;
}

Part::Part(PartConstructParams params): BasePart(&TYPE, params) {                      
    _lastShape = shape;
    _lastSize = size;
}

void Part::updateCollider(rp::PhysicsCommon* common) {
    rp::CollisionShape* physShape;
    if (shape == PartType::Ball) {
        physShape = common->createSphereShape(glm::min(size.X(), size.Y(), size.Z()) * 0.5f);
    } else if (shape == PartType::Block) {
        physShape = common->createBoxShape(glmToRp(size * glm::vec3(0.5f)));
    }

    // Recreate the rigidbody if the shape changes
    if (rigidBody->getNbColliders() > 0 && (_lastShape != shape || _lastSize != size)) {
        // TODO: This causes Touched to get called twice. Fix this.
        rigidBody->removeCollider(rigidBody->getCollider(0));
        rigidBody->addCollider(physShape, rp::Transform());
    }

    if (rigidBody->getNbColliders() == 0)
        rigidBody->addCollider(physShape, rp::Transform());


    _lastShape = shape;
    _lastSize = size;
}

Vector3 Part::GetEffectiveSize() {
    return shape == PartType::Ball ? (Vector3)glm::vec3(glm::min(size.X(), size.Y(), size.Z())) : size;

}