#pragma once

#include "datatypes/vector.h"
#include "enum/part.h"
#include "reactphysics3d/body/RigidBody.h"
#include "utils.h"
#include <functional>
#include <list>
#include <memory>
#include <reactphysics3d/reactphysics3d.h>

namespace rp = reactphysics3d;

class BasePart;
class PhysWorld;
class PhysicsEventListener : public rp::EventListener {
    friend PhysWorld;
    PhysWorld* world;

    PhysicsEventListener(PhysWorld*);

    void onContact(const rp::CollisionCallback::CallbackData&) override;
    void onTrigger(const rp::OverlapCallback::CallbackData&) override;
};

struct PhysJointInfo { virtual ~PhysJointInfo() = default; protected: PhysJointInfo() = default; };
struct PhysJointGlueInfo : PhysJointInfo { Vector3 anchorPoint; inline PhysJointGlueInfo(Vector3 anchorPoint) : anchorPoint(anchorPoint) {} };
struct PhysJointWeldInfo : PhysJointInfo { Vector3 anchorPoint; inline PhysJointWeldInfo(Vector3 anchorPoint) : anchorPoint(anchorPoint) {} };
struct PhysJointSnapInfo : PhysJointInfo { Vector3 anchorPoint; inline PhysJointSnapInfo(Vector3 anchorPoint) : anchorPoint(anchorPoint) {} };
struct PhysJointHingeInfo : PhysJointInfo { Vector3 anchorPoint; Vector3 rotationAxis; inline PhysJointHingeInfo(Vector3 anchorPoint, Vector3 rotationAxis) : anchorPoint(anchorPoint), rotationAxis(rotationAxis) {} };
struct PhysJointMotorInfo : PhysJointInfo { Vector3 anchorPoint; Vector3 rotationAxis; inline PhysJointMotorInfo(Vector3 anchorPoint, Vector3 rotationAxis) : anchorPoint(anchorPoint), rotationAxis(rotationAxis) {} };

class PhysWorld;
class PhysJoint {
    rp::Joint* joint;
    inline PhysJoint(rp::Joint* joint) : joint(joint) {}
    friend PhysWorld;
public:
    inline PhysJoint() {}
};

struct RaycastResult;
class PhysRigidBody {
    rp::RigidBody* rigidBody = nullptr;
    inline PhysRigidBody(rp::RigidBody* rigidBody) : rigidBody(rigidBody) {}
    Vector3 _lastSize;
    PartType _lastShape;

    friend PhysWorld;
    friend RaycastResult;
public:
    inline PhysRigidBody() {}

    inline void setActive(bool active) { if (!rigidBody) return; rigidBody->setIsActive(active); }
    inline void setCollisionsEnabled(bool enabled) { if (!rigidBody) return; rigidBody->getCollider(0)->setIsWorldQueryCollider(enabled); }
    void updateCollider(std::shared_ptr<BasePart>);
};

// // Provides internal implementation-specific values from the raycast result
// struct RaycastResultInternal {
//     rp::decimal hitFraction;
//     rp::Collider* collider;
// };

struct RaycastResult {
    Vector3 worldPoint;
    Vector3 worldNormal;
    int triangleIndex;
    PhysRigidBody body;
    nullable std::shared_ptr<BasePart> hitPart;

    RaycastResult(const rp::RaycastInfo& raycastInfo);
};

enum FilterResult {
    TARGET, // The object is captured
    BLOCK, // The object blocks any objects behind it, but is not captured
    PASS, // The object is transparent, ignore it
};

class BasePart;
typedef std::function<FilterResult(std::shared_ptr<BasePart>)> RaycastFilter;

class PhysWorld : public std::enable_shared_from_this<PhysWorld> {
    rp::PhysicsWorld* worldImpl;
    PhysicsEventListener physicsEventListener;
    std::list<std::shared_ptr<BasePart>> simulatedBodies;

public:
    PhysWorld();
    ~PhysWorld();

    void step(float deltaTime);
    
    void addBody(std::shared_ptr<BasePart>);
    void removeBody(std::shared_ptr<BasePart>);

    PhysJoint createJoint(PhysJointInfo& type, std::shared_ptr<BasePart> part0, std::shared_ptr<BasePart> part1);
    void destroyJoint(PhysJoint joint);

    inline const std::list<std::shared_ptr<BasePart>>& getSimulatedBodies() { return simulatedBodies; }
    void syncBodyProperties(std::shared_ptr<BasePart>);
    std::optional<const RaycastResult> castRay(Vector3 point, Vector3 rotation, float maxLength, std::optional<RaycastFilter> filter, unsigned short categoryMaskBits);
};