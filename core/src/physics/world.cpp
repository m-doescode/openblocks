#include "world.h"
#include "datatypes/variant.h"
#include "datatypes/vector.h"
#include "objects/joint/jointinstance.h"
#include "objects/part/basepart.h"
#include "physics/util.h"
#include <reactphysics3d/constraint/FixedJoint.h>
#include "reactphysics3d/constraint/HingeJoint.h"
#include "timeutil.h"
#include <memory>
#include "objects/service/workspace.h"

rp::PhysicsCommon physicsCommon;

PhysWorld::PhysWorld() : physicsEventListener(this) {
    worldImpl = physicsCommon.createPhysicsWorld();

    worldImpl->setGravity(rp::Vector3(0, -196.2, 0));
    // world->setContactsPositionCorrectionTechnique(rp::ContactsPositionCorrectionTechnique::BAUMGARTE_CONTACTS);
    // physicsWorld->setNbIterationsPositionSolver(2000);
    // physicsWorld->setNbIterationsVelocitySolver(2000);
    // physicsWorld->setSleepLinearVelocity(10);
    // physicsWorld->setSleepAngularVelocity(5);

    worldImpl->setEventListener(&physicsEventListener);

}

PhysWorld::~PhysWorld() {
    physicsCommon.destroyPhysicsWorld(worldImpl);    
}

PhysicsEventListener::PhysicsEventListener(PhysWorld* world) : world(world) {}

void PhysicsEventListener::onContact(const rp::CollisionCallback::CallbackData& data) {
    for (size_t i = 0; i < data.getNbContactPairs(); i++) {
        auto pair = data.getContactPair(i);
        auto type = pair.getEventType();
        if (type == rp::CollisionCallback::ContactPair::EventType::ContactStay) continue;

        auto part0 = reinterpret_cast<BasePart*>(pair.getBody1()->getUserData())->shared<BasePart>();
        auto part1 = reinterpret_cast<BasePart*>(pair.getBody2()->getUserData())->shared<BasePart>();

        if (type == reactphysics3d::CollisionCallback::ContactPair::EventType::ContactStart) {
            part0->Touched->Fire({ (Variant)InstanceRef(part1) });
            part1->Touched->Fire({ (Variant)InstanceRef(part0) });
        } else if (type == reactphysics3d::CollisionCallback::ContactPair::EventType::ContactExit) {
            part0->TouchEnded->Fire({ (Variant)InstanceRef(part1) });
            part1->TouchEnded->Fire({ (Variant)InstanceRef(part0) });
        }
    }
}

void PhysicsEventListener::onTrigger(const rp::OverlapCallback::CallbackData& data) {
    for (size_t i = 0; i < data.getNbOverlappingPairs(); i++) {
        auto pair = data.getOverlappingPair(i);
        auto type = pair.getEventType();
        if (type == rp::OverlapCallback::OverlapPair::EventType::OverlapStay) continue;

        auto part0 = reinterpret_cast<BasePart*>(pair.getBody1()->getUserData())->shared<BasePart>();
        auto part1 = reinterpret_cast<BasePart*>(pair.getBody2()->getUserData())->shared<BasePart>();

        if (type == reactphysics3d::OverlapCallback::OverlapPair::EventType::OverlapStart) {
            part0->Touched->Fire({ (Variant)InstanceRef(part1) });
            part1->Touched->Fire({ (Variant)InstanceRef(part0) });
        } else if (type == reactphysics3d::OverlapCallback::OverlapPair::EventType::OverlapExit) {
            part0->TouchEnded->Fire({ (Variant)InstanceRef(part1) });
            part1->TouchEnded->Fire({ (Variant)InstanceRef(part0) });
        }
    }
}

tu_time_t physTime;
void PhysWorld::step(float deltaTime) {
    tu_time_t startTime = tu_clock_micros();

    worldImpl->update(std::min(deltaTime / 2, (1/60.f)));

    // TODO: Add list of tracked parts in workspace based on their ancestry using inWorkspace property of Instance
    for (std::shared_ptr<BasePart> part : simulatedBodies) {
        if (!part->rigidBody) continue;

        // Sync properties
        const rp::Transform& transform = part->rigidBody->getTransform();
        part->cframe = CFrame(transform);
        part->velocity = part->rigidBody->getLinearVelocity();

        // part->rigidBody->enableGravity(true);
        // RotateV/Motor joint
        for (auto& joint : part->secondaryJoints) {
            if (joint.expired() || !joint.lock()->IsA("RotateV")) continue;
                        
            std::shared_ptr<JointInstance> motor = joint.lock()->CastTo<JointInstance>().expect();
            float rate = motor->part0.lock()->GetSurfaceParamB(-motor->c0.LookVector().Unit()) * 30;
            // part->rigidBody->enableGravity(false);
            part->rigidBody->setAngularVelocity(-(motor->part0.lock()->cframe * motor->c0).LookVector() * rate);
        }
    }

    physTime = tu_clock_micros() - startTime;
}
    
void PhysWorld::addBody(std::shared_ptr<BasePart> part) {
    simulatedBodies.push_back(part);
}

void PhysWorld::removeBody(std::shared_ptr<BasePart> part) {
    auto it = std::find(simulatedBodies.begin(), simulatedBodies.end(), part);
    simulatedBodies.erase(it);
}

void PhysWorld::syncBodyProperties(std::shared_ptr<BasePart> part) {
    rp::Transform transform = part->cframe;
    if (!part->rigidBody) {
        part->rigidBody = worldImpl->createRigidBody(transform);
    } else {
        part->rigidBody->setTransform(transform);
    }

    part->updateCollider(&physicsCommon);

    part->rigidBody->setType(part->anchored ? rp::BodyType::STATIC : rp::BodyType::DYNAMIC);
    part->rigidBody->getCollider(0)->setCollisionCategoryBits(0b11);

    part->rigidBody->getCollider(0)->setIsSimulationCollider(part->canCollide);
    part->rigidBody->getCollider(0)->setIsTrigger(!part->canCollide);

    rp::Material& material = part->rigidBody->getCollider(0)->getMaterial();
    material.setFrictionCoefficient(0.35);
    material.setMassDensity(1.f);

    //https://github.com/DanielChappuis/reactphysics3d/issues/170#issuecomment-691514860
    part->rigidBody->updateMassFromColliders();
    part->rigidBody->updateLocalInertiaTensorFromColliders();

    part->rigidBody->setLinearVelocity(part->velocity);
    // part->rigidBody->setMass(density * part->size.x * part->size.y * part->size.z);

    part->rigidBody->setUserData(&*part);
}

// Ray-casting


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

        std::shared_ptr<BasePart> part = partFromBody(raycastInfo.body);
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

std::optional<const RaycastResult> PhysWorld::castRay(Vector3 point, Vector3 rotation, float maxLength, std::optional<RaycastFilter> filter, unsigned short categoryMaskBits) {
    // std::scoped_lock lock(globalPhysicsLock);
    rp::Ray ray(glmToRp(point), glmToRp(glm::normalize((glm::vec3)rotation)) * maxLength);
    NearestRayHit rayHit(glmToRp(point), filter);
    worldImpl->raycast(ray, &rayHit, categoryMaskBits);
    return rayHit.getNearestHit();
}

PhysJoint PhysWorld::createJoint(PhysJointInfo& type, std::shared_ptr<BasePart> part0, std::shared_ptr<BasePart> part1) {
    // error checking
    if (part0->rigidBody == nullptr
        || part1->rigidBody == nullptr
        || !part0->workspace()
        || !part1->workspace()
        || part0->workspace()->physicsWorld != shared_from_this()
        || part1->workspace()->physicsWorld != shared_from_this()
    ) { Logger::fatalError("Failed to create joint between two parts due to the call being invalid"); panic(); };

    if (PhysJointGlueInfo* info = dynamic_cast<PhysJointGlueInfo*>(&type)) {
        return worldImpl->createJoint(rp::FixedJointInfo(part0->rigidBody, part1->rigidBody, info->anchorPoint));
    } else if (PhysJointWeldInfo* info = dynamic_cast<PhysJointWeldInfo*>(&type)) {
        return worldImpl->createJoint(rp::FixedJointInfo(part0->rigidBody, part1->rigidBody, info->anchorPoint));
    } else if (PhysJointSnapInfo* info = dynamic_cast<PhysJointSnapInfo*>(&type)) {
        return worldImpl->createJoint(rp::FixedJointInfo(part0->rigidBody, part1->rigidBody, info->anchorPoint));
    } else if (PhysJointHingeInfo* info = dynamic_cast<PhysJointHingeInfo*>(&type)) {
        return worldImpl->createJoint(rp::HingeJointInfo(part0->rigidBody, part1->rigidBody, info->anchorPoint, info->rotationAxis));
    } else if (PhysJointMotorInfo* info = dynamic_cast<PhysJointMotorInfo*>(&type)) {
        auto implInfo = rp::HingeJointInfo(part0->rigidBody, part1->rigidBody, info->anchorPoint, info->rotationAxis);
        implInfo.isCollisionEnabled = false;
        return worldImpl->createJoint(implInfo);

        // part1.lock()->rigidBody->getCollider(0)->setCollideWithMaskBits(0b10);
        // part1.lock()->rigidBody->getCollider(0)->setCollisionCategoryBits(0b10);
        // part0.lock()->rigidBody->getCollider(0)->setCollideWithMaskBits(0b01);
        // part0.lock()->rigidBody->getCollider(0)->setCollisionCategoryBits(0b01);
    }

    panic(); // Unreachable
}

void PhysWorld::destroyJoint(PhysJoint joint) {

}