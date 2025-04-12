#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "datatypes/cframe.h"
#include "datatypes/color3.h"
#include "datatypes/vector.h"
#include "objects/base/instance.h"
#include "rendering/surface.h"
#include <reactphysics3d/reactphysics3d.h>

namespace rp = reactphysics3d;

// For easy construction from C++. Maybe should be removed?
struct PartConstructParams {
    glm::vec3 position;
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 size;
    Data::Color3 color;
    
    bool anchored = false;
    bool locked = false;
};

class Part : public Instance {
protected:
    void OnAncestryChanged(std::optional<std::shared_ptr<Instance>> child, std::optional<std::shared_ptr<Instance>> newParent) override;
    void onUpdated(std::string);
public:
    const static InstanceType TYPE;

    Data::CFrame cframe;
    glm::vec3 size;
    Data::Color3 color;
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

    inline Data::Vector3 position() { return cframe.Position(); }

    // Calculate size of axis-aligned bounding box
    Data::Vector3 GetAABB();
};