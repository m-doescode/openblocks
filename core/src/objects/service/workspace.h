#pragma once

#include "objectmodel/macro.h"
#include "objects/base/service.h"
#include "physics/world.h"
#include "rendering/frustum.h"
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <mutex>
#include <queue>

class BasePart;
class Snap;
class Weld;
class Rotate;
class RotateV;

struct ContactItem {
    std::shared_ptr<BasePart> part0;
    std::shared_ptr<BasePart> part1;
    enum {
        CONTACTITEM_TOUCHED,
        CONTACTITEM_TOUCHENDED,
    } action;
};

class Workspace : public Service {
    INSTANCE_HEADER
    
    std::queue<ContactItem> contactQueue;
    std::mutex contactQueueLock;

    std::shared_ptr<PhysWorld> physicsWorld;
    friend PhysWorld;
protected:
    bool initialized = false;

public:
    Workspace();
    ~Workspace();
    
    void InitService() override;
    void OnRun() override;

    std::recursive_mutex queueLock;

    float fallenPartsDestroyHeight = -500;

    // static inline std::shared_ptr<Workspace> New() { return std::make_shared<Workspace>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Workspace>(); };

    inline void AddBody(std::shared_ptr<BasePart> part) { physicsWorld->addBody(part); }
    inline void RemoveBody(std::shared_ptr<BasePart> part) { physicsWorld->removeBody(part); }
    void SyncPartPhysics(std::shared_ptr<BasePart> part);

    inline PhysJoint CreateJoint(PhysJointInfo& info, std::shared_ptr<BasePart> part0, std::shared_ptr<BasePart> part1) { return physicsWorld->createJoint(info, part0, part1); }
    inline void DestroyJoint(PhysJoint joint) { physicsWorld->destroyJoint(joint); }

    inline void TrackDrivenJoint(std::shared_ptr<JointInstance> motor) { return physicsWorld->trackDrivenJoint(motor); }
    inline void UntrackDrivenJoint(std::shared_ptr<JointInstance> motor) { return physicsWorld->untrackDrivenJoint(motor); }

    void PhysicsStep(float deltaTime);
    inline std::optional<const RaycastResult> CastRayNearest(glm::vec3 point, glm::vec3 rotation, float maxLength, std::optional<RaycastFilter> filter = std::nullopt, unsigned short categoryMaskBits = 0xFFFF) { return physicsWorld->castRay(point, rotation, maxLength, filter, categoryMaskBits); }
    std::vector<std::shared_ptr<Instance>> CastFrustum(Frustum frustum);
};