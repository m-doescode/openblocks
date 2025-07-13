#pragma once

#include "objects/annotation.h"
#include "objects/base/service.h"
#include "utils.h"
#include <glm/ext/vector_float3.hpp>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <reactphysics3d/body/RigidBody.h>
#include <reactphysics3d/engine/EventListener.h>
#include <reactphysics3d/engine/PhysicsCommon.h>
#include <reactphysics3d/engine/PhysicsWorld.h>

namespace rp = reactphysics3d;

struct RaycastResult {
    rp::Vector3 worldPoint;
    rp::Vector3 worldNormal;
    rp::decimal hitFraction;
    int triangleIndex;
    rp::Body* body;
    rp::Collider* collider;

    RaycastResult(const rp::RaycastInfo& raycastInfo);
};

enum FilterResult {
    TARGET, // The object is captured
    BLOCK, // The object blocks any objects behind it, but is not captured
    PASS, // The object is transparent, ignore it
};

class BasePart;
class Snap;
class Weld;
class Rotate;
class RotateV;

#ifndef __SIMULATION_TICKET
#define __SIMULATION_TICKET
typedef std::list<std::shared_ptr<BasePart>>::iterator SimulationTicket;
#endif

typedef std::function<FilterResult(std::shared_ptr<BasePart>)> RaycastFilter;

struct QueueItem {
    std::shared_ptr<BasePart> part;
    enum {
        QUEUEITEM_ADD,
        QUEUEITEM_REMOVE,
    } action;
};

struct ContactItem {
    std::shared_ptr<BasePart> part0;
    std::shared_ptr<BasePart> part1;
    enum {
        CONTACTITEM_TOUCHED,
        CONTACTITEM_TOUCHENDED,
    } action;
};

class Workspace;
class PhysicsEventListener : public rp::EventListener {
    friend Workspace;
    Workspace* workspace;

    PhysicsEventListener(Workspace*);

    void onContact(const rp::CollisionCallback::CallbackData&) override;
    void onTrigger(const rp::OverlapCallback::CallbackData&) override;
};

class DEF_INST_SERVICE_(explorer_icon="workspace") Workspace : public Service {
    AUTOGEN_PREAMBLE
    
    friend PhysicsEventListener;
    
    std::list<std::shared_ptr<BasePart>> simulatedBodies;
    std::list<QueueItem> bodyQueue;
    std::queue<ContactItem> contactQueue;
    std::mutex contactQueueLock;
    rp::PhysicsWorld* physicsWorld;
    static rp::PhysicsCommon* physicsCommon;
    PhysicsEventListener physicsEventListener;

    void updatePartPhysics(std::shared_ptr<BasePart> part);
protected:
    void InitService() override;
    bool initialized = false;

public:
    Workspace();
    ~Workspace();

    std::mutex globalPhysicsLock;
    std::recursive_mutex queueLock;

    DEF_PROP float fallenPartsDestroyHeight = -500;

    // static inline std::shared_ptr<Workspace> New() { return std::make_shared<Workspace>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Workspace>(); };

    void AddBody(std::shared_ptr<BasePart> part);
    void RemoveBody(std::shared_ptr<BasePart> part);
    void DestroyRigidBody(rp::RigidBody* rigidBody);
    void SyncPartPhysics(std::shared_ptr<BasePart> part);

    rp::Joint* CreateJoint(const rp::JointInfo& jointInfo);
    void DestroyJoint(rp::Joint* joint);

    void ProcessContactEvents();
    void PhysicsStep(float deltaTime);
    std::optional<const RaycastResult> CastRayNearest(glm::vec3 point, glm::vec3 rotation, float maxLength, std::optional<RaycastFilter> filter = std::nullopt, unsigned short categoryMaskBits = 0xFFFF);
};