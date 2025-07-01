#include "workspace.h"
#include "datatypes/variant.h"
#include "datatypes/ref.h"
#include "datatypes/vector.h"
#include "logger.h"
#include "objects/base/instance.h"
#include "objects/part.h"
#include "objects/service/jointsservice.h"
#include "objects/joint/jointinstance.h"
#include "objects/datamodel.h"
#include "physics/util.h"
#include "timeutil.h"
#include <memory>
#include <reactphysics3d/collision/CollisionCallback.h>
#include <reactphysics3d/collision/OverlapCallback.h>
#include <reactphysics3d/engine/PhysicsCommon.h>

rp::PhysicsCommon* Workspace::physicsCommon = new rp::PhysicsCommon;

Workspace::Workspace(): Service(&TYPE), physicsEventListener(this) {
    physicsWorld = physicsCommon->createPhysicsWorld();
}

Workspace::~Workspace() {
    if (physicsCommon)
        physicsCommon->destroyPhysicsWorld(physicsWorld);
}

PhysicsEventListener::PhysicsEventListener(Workspace* parent) : workspace(parent) {}

void PhysicsEventListener::onContact(const rp::CollisionCallback::CallbackData& data) {
    for (size_t i = 0; i < data.getNbContactPairs(); i++) {
        auto pair = data.getContactPair(i);
        auto type = pair.getEventType();
        if (type == rp::CollisionCallback::ContactPair::EventType::ContactStay) continue;

        auto part0 = reinterpret_cast<Part*>(pair.getBody1()->getUserData())->shared<Part>();
        auto part1 = reinterpret_cast<Part*>(pair.getBody2()->getUserData())->shared<Part>();

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

        auto part0 = reinterpret_cast<Part*>(pair.getBody1()->getUserData())->shared<Part>();
        auto part1 = reinterpret_cast<Part*>(pair.getBody2()->getUserData())->shared<Part>();

        if (type == reactphysics3d::OverlapCallback::OverlapPair::EventType::OverlapStart) {
            part0->Touched->Fire({ (Variant)InstanceRef(part1) });
            part1->Touched->Fire({ (Variant)InstanceRef(part0) });
        } else if (type == reactphysics3d::OverlapCallback::OverlapPair::EventType::OverlapExit) {
            part0->TouchEnded->Fire({ (Variant)InstanceRef(part1) });
            part1->TouchEnded->Fire({ (Variant)InstanceRef(part0) });
        }
    }
}

void Workspace::InitService() {
    if (initialized) return;
    initialized = true;

    physicsWorld->setGravity(rp::Vector3(0, -196.2, 0));
    // world->setContactsPositionCorrectionTechnique(rp3d::ContactsPositionCorrectionTechnique::BAUMGARTE_CONTACTS);
    physicsWorld->setNbIterationsPositionSolver(2000);
    physicsWorld->setNbIterationsVelocitySolver(2000);
    // physicsWorld->setSleepLinearVelocity(10);
    // physicsWorld->setSleepAngularVelocity(5);

    physicsWorld->setEventListener(&physicsEventListener);

    // Sync all parts
    for (auto it = this->GetDescendantsStart(); it != this->GetDescendantsEnd(); it++) {
        std::shared_ptr<Instance> obj = *it;
        if (!obj->IsA<Part>()) continue;
        std::shared_ptr<Part> part = obj->CastTo<Part>().expect();
        this->SyncPartPhysics(part);
        part->MakeJoints();
    }

    // Activate all joints
    for (auto it = this->GetDescendantsStart(); it != this->GetDescendantsEnd(); it++) {
        std::shared_ptr<Instance> obj = *it;
        if (!obj->IsA<JointInstance>()) continue;
        std::shared_ptr<JointInstance> joint = obj->CastTo<JointInstance>().expect();
        joint->UpdateProperty("Part0");
    }

    for (auto obj : dataModel().value()->GetService<JointsService>()->GetChildren()) {
        if (!obj->IsA<JointInstance>()) continue;
        std::shared_ptr<JointInstance> joint = obj->CastTo<JointInstance>().expect();
        joint->UpdateProperty("Part0");
    }
}

void Workspace::SyncPartPhysics(std::shared_ptr<Part> part) {
    printf("SyncPartPhysics-lck\n");
    std::scoped_lock lock(globalPhysicsLock);
    printf("SyncPartPhysics-post\n");

    rp::Transform transform = part->cframe;
    if (!part->rigidBody) {
        part->rigidBody = physicsWorld->createRigidBody(transform);
    } else {
        part->rigidBody->setTransform(transform);
    }

    rp::BoxShape* shape = physicsCommon->createBoxShape(glmToRp(part->size * glm::vec3(0.5f)));

    // Recreate the rigidbody if the shape changes
    if (part->rigidBody->getNbColliders() > 0
        && dynamic_cast<rp::BoxShape*>(part->rigidBody->getCollider(0)->getCollisionShape())->getHalfExtents() != shape->getHalfExtents()) {
        // TODO: This causes Touched to get called twice. Fix this.
        part->rigidBody->removeCollider(part->rigidBody->getCollider(0));
        part->rigidBody->addCollider(shape, rp::Transform());
    }

    if (part->rigidBody->getNbColliders() == 0)
        part->rigidBody->addCollider(shape, rp::Transform());

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

tu_time_t physTime;
void Workspace::PhysicsStep(float deltaTime) {
    tu_time_t startTime = tu_clock_micros();

    std::scoped_lock lock(globalPhysicsLock);
    physicsWorld->update(std::min(deltaTime / 2, (1/60.f)));

    // Update queued objects
    queueLock.lock();
    for (QueueItem item : bodyQueue) {
        if (item.action == QueueItem::QUEUEITEM_ADD) {
            simulatedBodies.push_back(item.part);
            item.part->simulationTicket = --simulatedBodies.end();
        } else if (item.part->simulationTicket->get() != nullptr) {
            simulatedBodies.erase(item.part->simulationTicket);
            item.part->simulationTicket = {};
        }
    }
    queueLock.unlock();

    // TODO: Add list of tracked parts in workspace based on their ancestry using inWorkspace property of Instance
    for (std::shared_ptr<Part> part : simulatedBodies) {
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

        // Destroy fallen parts
        if (part->cframe.Position().Y() < this->fallenPartsDestroyHeight) {
            auto parent = part->GetParent();
            part->Destroy();

            // If the parent of the part is a Model, destroy it too
            if (parent.has_value() && parent.value()->IsA("Model"))
                parent.value()->Destroy();
        }
    }

    physTime = tu_clock_micros() - startTime;
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

std::optional<const RaycastResult> Workspace::CastRayNearest(glm::vec3 point, glm::vec3 rotation, float maxLength, std::optional<RaycastFilter> filter, unsigned short categoryMaskBits) {
    printf("Raycast-lck\n");
    std::scoped_lock lock(globalPhysicsLock);
    printf("Raycast-post\n");
    rp::Ray ray(glmToRp(point), glmToRp(glm::normalize(rotation)) * maxLength);
    NearestRayHit rayHit(glmToRp(point), filter);
    physicsWorld->raycast(ray, &rayHit, categoryMaskBits);
    return rayHit.getNearestHit();
}

void Workspace::DestroyRigidBody(rp::RigidBody* rigidBody) {
    std::scoped_lock lock(globalPhysicsLock);
    physicsWorld->destroyRigidBody(rigidBody);
}

void Workspace::DestroyJoint(rp::Joint* joint) {
    std::scoped_lock lock(globalPhysicsLock);
    physicsWorld->destroyJoint(joint);
}

rp::Joint* Workspace::CreateJoint(const rp::JointInfo& jointInfo) {
    std::scoped_lock lock(globalPhysicsLock);
    rp::Joint* joint = physicsWorld->createJoint(jointInfo);

    return joint;
}

void Workspace::AddBody(std::shared_ptr<Part> part) {
    queueLock.lock();
    bodyQueue.push_back({part, QueueItem::QUEUEITEM_ADD});
    queueLock.unlock();
}

void Workspace::RemoveBody(std::shared_ptr<Part> part) {
    queueLock.lock();
    bodyQueue.push_back({part, QueueItem::QUEUEITEM_REMOVE});
    queueLock.unlock();
}