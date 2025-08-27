#pragma once

#include "datatypes/vector.h"
#include "enum/part.h"
#include "utils.h"
#include <functional>
#include <list>
#include <memory>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Constraints/TwoBodyConstraint.h>

class BasePart;
class PhysWorld;

struct PhysJointInfo { virtual ~PhysJointInfo() = default; protected: PhysJointInfo() = default; };
struct PhysJointGlueInfo : PhysJointInfo { Vector3 anchorPoint; inline PhysJointGlueInfo(Vector3 anchorPoint) : anchorPoint(anchorPoint) {} };
struct PhysJointWeldInfo : PhysJointInfo { Vector3 anchorPoint; inline PhysJointWeldInfo(Vector3 anchorPoint) : anchorPoint(anchorPoint) {} };
struct PhysJointSnapInfo : PhysJointInfo { Vector3 anchorPoint; inline PhysJointSnapInfo(Vector3 anchorPoint) : anchorPoint(anchorPoint) {} };
struct PhysJointHingeInfo : PhysJointInfo { Vector3 anchorPoint; Vector3 rotationAxis; inline PhysJointHingeInfo(Vector3 anchorPoint, Vector3 rotationAxis) : anchorPoint(anchorPoint), rotationAxis(rotationAxis) {} };
struct PhysJointMotorInfo : PhysJointInfo { Vector3 anchorPoint; Vector3 rotationAxis; inline PhysJointMotorInfo(Vector3 anchorPoint, Vector3 rotationAxis) : anchorPoint(anchorPoint), rotationAxis(rotationAxis) {} };

class PhysWorld;
struct PhysJoint {
public:
    JPH::TwoBodyConstraint* jointImpl;
};

struct RaycastResult;
class PhysRigidBody {
    JPH::Body* bodyImpl = nullptr;
    inline PhysRigidBody(JPH::Body* rigidBody) : bodyImpl(rigidBody) {}
    Vector3 _lastSize;
    PartType _lastShape;
    bool collisionsEnabled = true;

    friend PhysWorld;
    friend RaycastResult;
public:
    inline PhysRigidBody() {}

    inline void setActive(bool active) { if (!bodyImpl) return; }
    inline void setCollisionsEnabled(bool enabled) { collisionsEnabled = enabled; }
    inline bool isCollisionsEnabled() { return collisionsEnabled; }
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
    PhysRigidBody body;
    nullable std::shared_ptr<BasePart> hitPart;
};

enum FilterResult {
    TARGET, // The object is captured
    BLOCK, // The object blocks any objects behind it, but is not captured
    PASS, // The object is transparent, ignore it
};

class BasePart;
typedef std::function<FilterResult(std::shared_ptr<BasePart>)> RaycastFilter;

class BroadPhaseLayerInterface : public JPH::BroadPhaseLayerInterface {
    uint GetNumBroadPhaseLayers() const override;
    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;
    const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;
};

class ObjectBroadPhaseFilter : public JPH::ObjectVsBroadPhaseLayerFilter {
    bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
};

class ObjectLayerPairFilter : public JPH::ObjectLayerPairFilter {
    bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override;
};

class PhysWorld : public std::enable_shared_from_this<PhysWorld> {
    BroadPhaseLayerInterface broadPhaseLayerInterface;
    ObjectBroadPhaseFilter objectBroadPhasefilter;
    ObjectLayerPairFilter objectLayerPairFilter;
    JPH::PhysicsSystem worldImpl;
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

void physicsInit();
void physicsDeinit();