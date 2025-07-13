#include "wedgepart.h"
#include "physics/util.h"

WedgePart::WedgePart(): BasePart(&TYPE) {
}

WedgePart::WedgePart(PartConstructParams params): BasePart(&TYPE, params) {                      
    
}

void WedgePart::updateCollider(rp::PhysicsCommon* common) {
    rp::BoxShape* shape = common->createBoxShape(glmToRp(size * glm::vec3(0.5f)));

    // Recreate the rigidbody if the shape changes
    if (rigidBody->getNbColliders() > 0
        && dynamic_cast<rp::BoxShape*>(rigidBody->getCollider(0)->getCollisionShape())->getHalfExtents() != shape->getHalfExtents()) {
        // TODO: This causes Touched to get called twice. Fix this.
        rigidBody->removeCollider(rigidBody->getCollider(0));
        rigidBody->addCollider(shape, rp::Transform());
    }

    if (rigidBody->getNbColliders() == 0)
        rigidBody->addCollider(shape, rp::Transform());
}