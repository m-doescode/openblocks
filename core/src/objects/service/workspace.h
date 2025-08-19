#pragma once

#include "objects/annotation.h"
#include "objects/base/service.h"
#include "physics/world.h"
#include "utils.h"
#include <glm/ext/vector_float3.hpp>
#include <list>
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

class DEF_INST_SERVICE_(explorer_icon="workspace") Workspace : public Service {
    AUTOGEN_PREAMBLE
    
    std::queue<ContactItem> contactQueue;
    std::mutex contactQueueLock;

    std::shared_ptr<PhysWorld> physicsWorld;
protected:
    void InitService() override;
    void OnRun() override;
    bool initialized = false;

public:
    Workspace();
    ~Workspace();

    std::recursive_mutex queueLock;

    DEF_PROP float fallenPartsDestroyHeight = -500;

    // static inline std::shared_ptr<Workspace> New() { return std::make_shared<Workspace>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Workspace>(); };

    inline void AddBody(std::shared_ptr<BasePart> part) { physicsWorld->addBody(part); }
    inline void RemoveBody(std::shared_ptr<BasePart> part) { physicsWorld->removeBody(part); }
    void SyncPartPhysics(std::shared_ptr<BasePart> part);

    rp::Joint* CreateJoint(const rp::JointInfo& jointInfo);
    void DestroyJoint(rp::Joint* joint);

    void PhysicsStep(float deltaTime);
    inline std::optional<const RaycastResult> CastRayNearest(glm::vec3 point, glm::vec3 rotation, float maxLength, std::optional<RaycastFilter> filter = std::nullopt, unsigned short categoryMaskBits = 0xFFFF) { return physicsWorld->castRay(point, rotation, maxLength, filter, categoryMaskBits); }
};