#pragma once

#include "base.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "../rendering/material.h"
#include <reactphysics3d/reactphysics3d.h>

namespace rp = reactphysics3d;

// For easy construction from C++. Maybe should be removed?
struct PartConstructParams {
    glm::vec3 position;
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale;
    Material material;
    
    bool anchored = false;
};

class Part : public Instance {
protected:
    void OnParentUpdated(std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent) override;
    void onUpdated(std::string);
public:
    static InstanceType* TYPE;

    // TODO: Switch these over to our dedicated datatypes
    glm::vec3 position;
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale;
    Material material;
    
    bool anchored = false;
    rp::RigidBody* rigidBody = nullptr;
    
    Part();
    Part(PartConstructParams params);
    ~Part() override;

    static inline std::shared_ptr<Part> New() { return std::make_shared<Part>(); };
    static inline std::shared_ptr<Part> New(PartConstructParams params) { return std::make_shared<Part>(params); };
    static inline InstanceRef CreateGeneric() { return std::make_shared<Part>(); };
    virtual InstanceType* GetClass() override;
};