#include "workspace.h"
#include "objects/base/instance.h"
#include "physics/util.h"
#include <reactphysics3d/engine/PhysicsCommon.h>

const InstanceType Workspace::TYPE = {
    .super = &Instance::TYPE,
    .className = "Workspace",
    .constructor = &Workspace::Create,
    .explorerIcon = "workspace",
    .flags = INSTANCE_NOTCREATABLE | INSTANCE_SERVICE,
};

const InstanceType* Workspace::GetClass() {
    return &TYPE;
}

rp::PhysicsCommon* Workspace::physicsCommon = new rp::PhysicsCommon;

Workspace::Workspace(): Service(&TYPE) {
}

Workspace::~Workspace() {
    if (physicsWorld && physicsCommon)
        physicsCommon->destroyPhysicsWorld(physicsWorld);
}

void Workspace::InitService() {
    if (initialized) return;
    initialized = true;

    physicsWorld = physicsCommon->createPhysicsWorld();

    physicsWorld->setGravity(rp::Vector3(0, -196.2, 0));
    // world->setContactsPositionCorrectionTechnique(rp3d::ContactsPositionCorrectionTechnique::BAUMGARTE_CONTACTS);
    physicsWorld->setNbIterationsPositionSolver(2000);
    physicsWorld->setNbIterationsVelocitySolver(2000);

    // physicsWorld->setEventListener(&eventListener);

    // Sync all parts
    for (auto it = this->GetDescendantsStart(); it != this->GetDescendantsEnd(); it++) {
        InstanceRef obj = *it;
        if (obj->GetClass()->className != "Part") continue; // TODO: Replace this with a .IsA call instead of comparing the class name directly
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(obj);
        this->SyncPartPhysics(part);
    }
}

void Workspace::SyncPartPhysics(std::shared_ptr<Part> part) {
    if (!physicsWorld) return;

    glm::mat4 rotMat = glm::mat4(1.0f);

    rp::Transform transform = part->cframe;
    if (!part->rigidBody) {
        part->rigidBody = physicsWorld->createRigidBody(transform);
    } else {
        part->rigidBody->setTransform(transform);
    }

    rp::BoxShape* shape = physicsCommon->createBoxShape(glmToRp(part->size * glm::vec3(0.5f)));

    if (part->rigidBody->getNbColliders() > 0) {
        part->rigidBody->removeCollider(part->rigidBody->getCollider(0));
    }

    if (part->rigidBody->getNbColliders() == 0)
        part->rigidBody->addCollider(shape, rp::Transform());
    part->rigidBody->setType(part->anchored ? rp::BodyType::STATIC : rp::BodyType::DYNAMIC);
    part->rigidBody->getCollider(0)->setCollisionCategoryBits(0b11);

    part->rigidBody->setUserData(&*part);
}

void Workspace::PhysicsStep(float deltaTime) {
    // Step the simulation a few steps
    physicsWorld->update(std::min(deltaTime / 2, (1/60.f)));

    // Naive implementation. Parts are only considered so if they are just under Workspace
    // TODO: Add list of tracked parts in workspace based on their ancestry using inWorkspace property of Instance
    for (auto it = this->GetDescendantsStart(); it != this->GetDescendantsEnd(); it++) {
        InstanceRef obj = *it;
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

std::optional<const RaycastResult> Workspace::CastRayNearest(glm::vec3 point, glm::vec3 rotation, float maxLength, std::optional<RaycastFilter> filter, unsigned short categoryMaskBits) {
    rp::Ray ray(glmToRp(point), glmToRp(glm::normalize(rotation)) * maxLength);
    NearestRayHit rayHit(glmToRp(point), filter);
    physicsWorld->raycast(ray, &rayHit, categoryMaskBits);
    return rayHit.getNearestHit();
}

void Workspace::DestroyRigidBody(rp::RigidBody* rigidBody) {
    physicsWorld->destroyRigidBody(rigidBody);
}