#pragma once
#include <glm/glm.hpp>
#include <reactphysics3d/body/RigidBody.h>
#include "rendering/material.h"

namespace rp = reactphysics3d;

struct Part {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    Material material;
    
    bool anchored = false;
    rp::RigidBody* rigidBody = nullptr;

    void syncTransforms();
};