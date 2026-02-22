#pragma once

#include "datatypes/cframe.h"
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
class JointInstance;
class PhysWorld;

struct PhysJointInfo { virtual ~PhysJointInfo() = default; protected: PhysJointInfo() = default; };

struct PhysFixedJointInfo : PhysJointInfo {
    CFrame c0;
    CFrame c1;
    
    inline PhysFixedJointInfo(CFrame c0, CFrame c1) : c0(c0), c1(c1) {}
};

struct PhysRotatingJointInfo : PhysJointInfo {
    CFrame c0;
    CFrame c1;
    
    inline PhysRotatingJointInfo(CFrame c0, CFrame c1) : c0(c0), c1(c1) {}
};

struct PhysMotorizedJointInfo : PhysRotatingJointInfo {
    float initialVelocity;
    
    inline PhysMotorizedJointInfo(CFrame c0, CFrame c1, float initialVelocity) : PhysRotatingJointInfo(c0, c1), initialVelocity(initialVelocity) {}
};

struct PhysStepperJointInfo : PhysRotatingJointInfo {
    float initialAngle;
    float initialVelocity;
    
    inline PhysStepperJointInfo(CFrame c0, CFrame c1, float initialAngle, float initialVelocity) : PhysRotatingJointInfo(c0, c1), initialAngle(initialAngle), initialVelocity(initialVelocity) {}
};

class PhysWorld;
struct PhysJoint {
public:
    JPH::TwoBodyConstraint* jointImpl;
    PhysWorld* parentWorld;

    void setAngularVelocity(float velocity);
    void setTargetAngle(float angle);
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
    JPH::uint GetNumBroadPhaseLayers() const override;
    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;
#ifndef NO_JPH_LAYER_NAME
    const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;
#endif
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
    std::list<std::shared_ptr<JointInstance>> drivenJoints;

    friend PhysJoint;
public:
    PhysWorld();
    ~PhysWorld();

    void step(float deltaTime);
    
    void addBody(std::shared_ptr<BasePart>);
    void removeBody(std::shared_ptr<BasePart>);

    PhysJoint createJoint(PhysJointInfo& type, std::shared_ptr<BasePart> part0, std::shared_ptr<BasePart> part1);
    void destroyJoint(PhysJoint joint);

    void trackDrivenJoint(std::shared_ptr<JointInstance> motor);
    void untrackDrivenJoint(std::shared_ptr<JointInstance> motor);

    void setCFrameInternal(std::shared_ptr<BasePart> part, CFrame frame);

    inline const std::list<std::shared_ptr<BasePart>>& getSimulatedBodies() { return simulatedBodies; }
    void syncBodyProperties(std::shared_ptr<BasePart>);
    std::optional<const RaycastResult> castRay(Vector3 point, Vector3 rotation, float maxLength, std::optional<RaycastFilter> filter, unsigned short categoryMaskBits);
};

void physicsInit();
void physicsDeinit();