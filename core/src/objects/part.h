#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "datatypes/cframe.h"
#include "datatypes/color3.h"
#include "datatypes/vector.h"
#include "objects/base/instance.h"
#include "rendering/surface.h"
#include <optional>
#include <reactphysics3d/reactphysics3d.h>
#include <vector>
#include "annotation.h"

namespace rp = reactphysics3d;

// For easy construction from C++. Maybe should be removed?
struct PartConstructParams {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 size;
    Color3 color;
    
    bool anchored = false;
    bool locked = false;
};

class Snap;

class INSTANCE Part : public Instance {
protected:
    // Joints where this part is Part0
    std::vector<std::weak_ptr<JointInstance>> primaryJoints;
    // Joints where this part is Part1
    std::vector<std::weak_ptr<JointInstance>> secondaryJoints;

    void trackJoint(std::shared_ptr<JointInstance>);
    void untrackJoint(std::shared_ptr<JointInstance>);

    SurfaceType surfaceFromFace(NormalId);
    bool checkJointContinuity(std::shared_ptr<Part>);
    bool checkJointContinuityUp(std::shared_ptr<Part>);
    bool checkJointContinuityDown(std::shared_ptr<Part>);

    friend JointInstance;

    void OnAncestryChanged(std::optional<std::shared_ptr<Instance>> child, std::optional<std::shared_ptr<Instance>> newParent) override;
    void onUpdated(std::string);
public:
    const static InstanceType TYPE;

    Vector3 velocity;
    CFrame cframe;
    [[ def_prop(name="Size") ]]
    glm::vec3 size;
    [[ def_prop(name="Color") ]]
    Color3 color;
    [[ def_prop(name="Transparency") ]]
    float transparency = 0.f;
    bool selected = false;
    
    bool anchored = false;
    bool locked = false;
    rp::RigidBody* rigidBody = nullptr;

    SurfaceType topSurface = SurfaceType::SurfaceStuds;
    SurfaceType bottomSurface = SurfaceType::SurfaceInlets;
    SurfaceType leftSurface = SurfaceType::SurfaceSmooth;
    SurfaceType rightSurface = SurfaceType::SurfaceSmooth;
    SurfaceType frontSurface = SurfaceType::SurfaceSmooth;
    SurfaceType backSurface = SurfaceType::SurfaceSmooth;
    
    Part();
    Part(PartConstructParams params);
    ~Part() override;

    static inline std::shared_ptr<Part> New() { return std::make_shared<Part>(); };
    static inline std::shared_ptr<Part> New(PartConstructParams params) { return std::make_shared<Part>(params); };
    static inline InstanceRef CreateGeneric() { return std::make_shared<Part>(); };
    virtual const InstanceType* GetClass() override;

    inline Vector3 position() { return cframe.Position(); }

    void MakeJoints();
    void BreakJoints();

    // Calculate size of axis-aligned bounding box
    Vector3 GetAABB();
};