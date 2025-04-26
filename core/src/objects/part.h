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

class INSTANCE_WITH(explorer_icon="part") Part : public Instance {
    AUTOGEN_PREAMBLE
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
    [[ def_prop(name="Velocity", on_update=onUpdated) ]]
    Vector3 velocity;
    [[ def_prop(name="CFrame", on_update=onUpdated), cframe_position_prop(name="Position"), cframe_rotation_prop(name="Rotation") ]]
    CFrame cframe;
    [[ def_prop(name="Size", category=PART, on_update=onUpdated) ]]
    glm::vec3 size;
    [[ def_prop(name="Color", category=APPEARANCE) ]]
    Color3 color;
    [[ def_prop(name="Transparency", category=APPEARANCE) ]]
    float transparency = 0.f;
    bool selected = false;
    
    [[ def_prop(name="Anchored", category=BEHAVIOR, on_update=onUpdated) ]]
    bool anchored = false;
    [[ def_prop(name="Locked", category=BEHAVIOR) ]]
    bool locked = false;
    rp::RigidBody* rigidBody = nullptr;

    [[ def_prop(name="TopSurface", category=SURFACE) ]]
    SurfaceType topSurface = SurfaceType::SurfaceStuds;
    [[ def_prop(name="BottomSurface", category=SURFACE) ]]
    SurfaceType bottomSurface = SurfaceType::SurfaceInlets;
    [[ def_prop(name="LeftSurface", category=SURFACE) ]]
    SurfaceType leftSurface = SurfaceType::SurfaceSmooth;
    [[ def_prop(name="RightSurface", category=SURFACE) ]]
    SurfaceType rightSurface = SurfaceType::SurfaceSmooth;
    [[ def_prop(name="FrontSurface", category=SURFACE) ]]
    SurfaceType frontSurface = SurfaceType::SurfaceSmooth;
    [[ def_prop(name="BackSurface", category=SURFACE) ]]
    SurfaceType backSurface = SurfaceType::SurfaceSmooth;
    
    Part();
    Part(PartConstructParams params);
    ~Part() override;

    static inline std::shared_ptr<Part> New() { return std::make_shared<Part>(); };
    static inline std::shared_ptr<Part> New(PartConstructParams params) { return std::make_shared<Part>(params); };
    static inline InstanceRef Create() { return std::make_shared<Part>(); };

    inline Vector3 position() { return cframe.Position(); }

    void MakeJoints();
    void BreakJoints();

    // Calculate size of axis-aligned bounding box
    Vector3 GetAABB();
};