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
#include "physics/world.h"

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

class BasePart : public PVInstance {
    INSTANCE_HEADER
protected:
    // Joints where this part is Part0
    std::vector<std::weak_ptr<JointInstance>> primaryJoints;
    // Joints where this part is Part1
    std::vector<std::weak_ptr<JointInstance>> secondaryJoints;

    void trackJoint(std::shared_ptr<JointInstance>);
    void untrackJoint(std::shared_ptr<JointInstance>);

    SurfaceType surfaceFromFace(NormalId);
    bool checkJointContinuity(std::shared_ptr<BasePart>);
    bool checkJointContinuityUp(std::shared_ptr<BasePart>);
    bool checkJointContinuityDown(std::shared_ptr<BasePart>);
    bool checkSurfacesTouching(CFrame surfaceFrame, Vector3 size, Vector3 myFace, Vector3 otherFace, std::shared_ptr<BasePart> otherPart); 

    friend JointInstance;
    friend PhysWorld;

    virtual void OnWorkspaceAdded(nullable std::shared_ptr<Workspace> oldWorkspace, std::shared_ptr<Workspace> newWorkspace) override;
    virtual void OnWorkspaceRemoved(std::shared_ptr<Workspace> oldWorkspace) override;
    void OnAncestryChanged(nullable std::shared_ptr<Instance> child, nullable std::shared_ptr<Instance> newParent) override;
    void onUpdated(std::string, Variant, Variant);
    void onParamUpdated(std::string, Variant, Variant);

    BasePart();
    BasePart(PartConstructParams params);
public:
    Vector3 velocity;
    Vector3 rotVelocity;
    CFrame cframe;

    // Special compatibility changes for this property were made in
    // Instance::Serialize
    Vector3 size;

    Color3 color;
    float transparency = 0.f;
    float reflectance = 0.f;
    
    bool anchored = false;
    bool canCollide = true;
    bool locked = false;

    SurfaceType topSurface = SurfaceType::Studs;
    SurfaceType bottomSurface = SurfaceType::Inlet;
    SurfaceType leftSurface = SurfaceType::Smooth;
    SurfaceType rightSurface = SurfaceType::Smooth;
    SurfaceType frontSurface = SurfaceType::Smooth;
    SurfaceType backSurface = SurfaceType::Smooth;

    float topParamA = -0.5;
    float bottomParamA = -0.5;
    float leftParamA = -0.5;
    float rightParamA = -0.5;
    float frontParamA = -0.5;
    float backParamA = -0.5;

    float topParamB = 0.5;
    float bottomParamB = 0.5;
    float leftParamB = 0.5;
    float rightParamB = 0.5;
    float frontParamB = 0.5;
    float backParamB = 0.5;

    SignalSource Touched;
    SignalSource TouchEnded;

    PhysRigidBody rigidBody;
    
    inline SurfaceType GetSurfaceFromFace(NormalId face) { return surfaceFromFace(face); }
    float GetSurfaceParamA(Vector3 face);
    float GetSurfaceParamB(Vector3 face);
    virtual Vector3 GetEffectiveSize();

    ~BasePart() override;

    inline Vector3 position() { return cframe.Position(); }

    void MakeJoints();
    void BreakJoints();
    void UpdateNoBreakJoints();

    // Calculate size of axis-aligned bounding box
    Vector3 GetAABB();
};