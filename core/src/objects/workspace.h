#pragma once

#include "objects/annotation.h"
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
class Snap;
class Weld;
class Rotate;
class RotateV;

typedef std::function<FilterResult(std::shared_ptr<Part>)> RaycastFilter;

class DEF_INST_SERVICE_(explorer_icon="workspace") Workspace : public Service {
    AUTOGEN_PREAMBLE
    
    rp::PhysicsWorld* physicsWorld = nullptr;
    static rp::PhysicsCommon* physicsCommon; 

    friend Part;
    friend Snap;
    friend Weld;
    friend Rotate;
    friend RotateV;
protected:
    void InitService() override;
    bool initialized = false;

public:
    Workspace();
    ~Workspace();

    // static inline std::shared_ptr<Workspace> New() { return std::make_shared<Workspace>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Workspace>(); };

    void SyncPartPhysics(std::shared_ptr<Part> part);
    void DestroyRigidBody(rp::RigidBody* rigidBody);

    void PhysicsStep(float deltaTime);
    std::optional<const RaycastResult> CastRayNearest(glm::vec3 point, glm::vec3 rotation, float maxLength, std::optional<RaycastFilter> filter = std::nullopt, unsigned short categoryMaskBits = 0xFFFF);
};