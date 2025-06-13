#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "datatypes/cframe.h"
#include "datatypes/color3.h"
#include "datatypes/signal.h"
#include "datatypes/vector.h"
#include "objects/base/instance.h"
#include "enum/surface.h"
#include <optional>
#include <reactphysics3d/reactphysics3d.h>
#include <vector>
#include "annotation.h"
#include "objects/pvinstance.h"

namespace rp = reactphysics3d;

// For easy construction from C++. Maybe should be removed?
struct PartConstructParams {
    Vector3 position;
    Vector3 rotation;
    Vector3 size;
    Color3 color;
    
    bool anchored = false;
    bool locked = false;
};

class Workspace;

class DEF_INST_(explorer_icon="part") Part : public PVInstance {
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
    bool checkSurfacesTouching(CFrame surfaceFrame, Vector3 size, Vector3 myFace, Vector3 otherFace, std::shared_ptr<Part> otherPart); 

    friend JointInstance;
    friend Workspace;

    void OnAncestryChanged(std::optional<std::shared_ptr<Instance>> child, std::optional<std::shared_ptr<Instance>> newParent) override;
    void onUpdated(std::string);
public:
    DEF_PROP_CATEGORY(DATA)
    DEF_PROP_(on_update=onUpdated) Vector3 velocity;
    [[ def_prop(name="CFrame", on_update=onUpdated), cframe_position_prop(name="Position"), cframe_rotation_prop(name="Rotation") ]]
    CFrame cframe;

    DEF_PROP_CATEGORY(PART)
    // Special compatibility changes for this property were made in
    // Instance::Serialize
    DEF_PROP_(on_update=onUpdated) Vector3 size;

    DEF_PROP_CATEGORY(APPEARANCE)
    DEF_PROP Color3 color;
    DEF_PROP float transparency = 0.f;
    bool selected = false;
    
    DEF_PROP_CATEGORY(BEHAVIOR)
    DEF_PROP_(on_update=onUpdated) bool anchored = false;
    DEF_PROP_(on_update=onUpdated) bool canCollide = true;
    DEF_PROP bool locked = false;

    DEF_PROP_CATEGORY(SURFACE)
    DEF_PROP SurfaceType topSurface = SurfaceType::Studs;
    DEF_PROP SurfaceType bottomSurface = SurfaceType::Inlet;
    DEF_PROP SurfaceType leftSurface = SurfaceType::Smooth;
    DEF_PROP SurfaceType rightSurface = SurfaceType::Smooth;
    DEF_PROP SurfaceType frontSurface = SurfaceType::Smooth;
    DEF_PROP SurfaceType backSurface = SurfaceType::Smooth;

    DEF_PROP_CATEGORY(SURFACE_INPUT)
    DEF_PROP float topParamA = -0.5;
    DEF_PROP float bottomParamA = -0.5;
    DEF_PROP float leftParamA = -0.5;
    DEF_PROP float rightParamA = -0.5;
    DEF_PROP float frontParamA = -0.5;
    DEF_PROP float backParamA = -0.5;

    DEF_PROP float topParamB = 0.5;
    DEF_PROP float bottomParamB = 0.5;
    DEF_PROP float leftParamB = 0.5;
    DEF_PROP float rightParamB = 0.5;
    DEF_PROP float frontParamB = 0.5;
    DEF_PROP float backParamB = 0.5;

    DEF_SIGNAL SignalSource Touched;
    DEF_SIGNAL SignalSource TouchEnded;

    rp::RigidBody* rigidBody = nullptr;
    
    inline SurfaceType GetSurfaceFromFace(NormalId face) { return surfaceFromFace(face); }
    float GetSurfaceParamA(Vector3 face);
    float GetSurfaceParamB(Vector3 face);

    Part();
    Part(PartConstructParams params);
    ~Part() override;

    static inline std::shared_ptr<Part> New() { return std::make_shared<Part>(); };
    static inline std::shared_ptr<Part> New(PartConstructParams params) { return std::make_shared<Part>(params); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Part>(); };

    inline Vector3 position() { return cframe.Position(); }

    void MakeJoints();
    void BreakJoints();

    // Calculate size of axis-aligned bounding box
    Vector3 GetAABB();
};