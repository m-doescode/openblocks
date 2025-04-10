#pragma once

#include "objects/base/service.h"
#include <memory>
#include <reactphysics3d/body/RigidBody.h>
#include <reactphysics3d/engine/PhysicsCommon.h>
#include <reactphysics3d/engine/PhysicsWorld.h>

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

class Part;
typedef std::function<FilterResult(std::shared_ptr<Part>)> RaycastFilter;

class Workspace : public Service {
    rp::PhysicsWorld *physicsWorld = nullptr;

protected:
    void InitService() override;
    bool initialized = false;

public:
    const static InstanceType TYPE;

    Workspace();
    ~Workspace();

    // static inline std::shared_ptr<Workspace> New() { return std::make_shared<Workspace>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Workspace>(); };
    virtual const InstanceType* GetClass() override;

    void SyncPartPhysics(std::shared_ptr<Part> part);
    void DestroyRigidBody(rp::RigidBody* rigidBody);

    void PhysicsStep(float deltaTime);
    std::optional<const RaycastResult> CastRayNearest(glm::vec3 point, glm::vec3 rotation, float maxLength, std::optional<RaycastFilter> filter = std::nullopt, unsigned short categoryMaskBits = 0xFFFF);

};