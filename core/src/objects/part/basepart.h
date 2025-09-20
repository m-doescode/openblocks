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
#include <vector>
#include "objects/annotation.h"
#include "objects/pvinstance.h"
#include "physics/body.h"
#include "physics/world.h"

// Common macro for part properties
#define DEF_PROP_PHYS DEF_PROP_(on_update=onUpdated)
#define DEF_PROP_PHYSPARAM DEF_PROP_(on_update=onParamUpdated)

// For easy construction from C++. Maybe should be removed?
struct PartConstructParams {
    Vector3 position;
    Vector3 rotation;
    Vector3 size;
    Color3 color;
    
    bool anchored = false;
    bool locked = false;
};

class PhysWorld;

class DEF_INST_ABSTRACT_(explorer_icon="part") BasePart : public PVInstance {
    AUTOGEN_PREAMBLE
protected:
    std::vector<std::weak_ptr<JointInstance>> attachedJoints;

    SurfaceType surfaceFromFace(NormalId);
    // Ensures that "needle" is not already part of this assembly by traversing connected joints
    bool checkAttached(std::shared_ptr<BasePart> needle, std::shared_ptr<BasePart> prevPart = {});
    bool checkSurfacesTouching(CFrame surfaceFrame, Vector3 size, Vector3 myFace, Vector3 otherFace, std::shared_ptr<BasePart> otherPart); 

    friend JointInstance;
    friend PhysWorld;

    void OnAncestryChanged(nullable std::shared_ptr<Instance> child, nullable std::shared_ptr<Instance> newParent) override;
    void onUpdated(std::string);
    void onParamUpdated(std::string);

    BasePart(const InstanceType*);
    BasePart(const InstanceType*, PartConstructParams params);
public:
    DEF_PROP_CATEGORY(DATA)
    DEF_PROP_PHYS Vector3 velocity;
    [[ def_prop(name="CFrame", on_update=onUpdated), cframe_position_prop(name="Position"), cframe_rotation_prop(name="Rotation") ]]
    CFrame cframe;

    DEF_PROP_CATEGORY(PART)
    // Special compatibility changes for this property were made in
    // Instance::Serialize
    DEF_PROP_PHYS Vector3 size;

    DEF_PROP_CATEGORY(APPEARANCE)
    DEF_PROP Color3 color;
    DEF_PROP float transparency = 0.f;
    DEF_PROP float reflectance = 0.f;
    
    DEF_PROP_CATEGORY(BEHAVIOR)
    DEF_PROP_PHYS bool anchored = false;
    DEF_PROP_PHYS bool canCollide = true;
    DEF_PROP bool locked = false;

    DEF_PROP_CATEGORY(SURFACE)
    DEF_PROP SurfaceType topSurface = SurfaceType::Studs;
    DEF_PROP SurfaceType bottomSurface = SurfaceType::Inlet;
    DEF_PROP SurfaceType leftSurface = SurfaceType::Smooth;
    DEF_PROP SurfaceType rightSurface = SurfaceType::Smooth;
    DEF_PROP SurfaceType frontSurface = SurfaceType::Smooth;
    DEF_PROP SurfaceType backSurface = SurfaceType::Smooth;

    DEF_PROP_CATEGORY(SURFACE_INPUT)
    DEF_PROP_PHYSPARAM float topParamA = -0.5;
    DEF_PROP_PHYSPARAM float bottomParamA = -0.5;
    DEF_PROP_PHYSPARAM float leftParamA = -0.5;
    DEF_PROP_PHYSPARAM float rightParamA = -0.5;
    DEF_PROP_PHYSPARAM float frontParamA = -0.5;
    DEF_PROP_PHYSPARAM float backParamA = -0.5;

    DEF_PROP_PHYSPARAM float topParamB = 0.5;
    DEF_PROP_PHYSPARAM float bottomParamB = 0.5;
    DEF_PROP_PHYSPARAM float leftParamB = 0.5;
    DEF_PROP_PHYSPARAM float rightParamB = 0.5;
    DEF_PROP_PHYSPARAM float frontParamB = 0.5;
    DEF_PROP_PHYSPARAM float backParamB = 0.5;

    DEF_SIGNAL SignalSource Touched;
    DEF_SIGNAL SignalSource TouchEnded;

    PhysBodyHandle bodyHandle;
    
    inline SurfaceType GetSurfaceFromFace(NormalId face) { return surfaceFromFace(face); }
    float GetSurfaceParamA(Vector3 face);
    float GetSurfaceParamB(Vector3 face);
    virtual Vector3 GetEffectiveSize();

    ~BasePart() override;

    inline Vector3 position() { return cframe.Position(); }

    void MakeJoints();
    void BreakJoints();
    void UpdateNoBreakJoints();

    void AddJoint(std::shared_ptr<JointInstance>);
    void RemoveJoint(std::shared_ptr<JointInstance>);

    // Calculate size of axis-aligned bounding box
    Vector3 GetAABB();
};

#undef DEF_PROP_PHYS
#undef DEF_PROP_PHYSPARAM