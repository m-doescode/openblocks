#pragma once
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <reactphysics3d/body/RigidBody.h>
#include "rendering/material.h"

namespace rp = reactphysics3d;

struct Part {
    glm::vec3 position;
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale;
    Material material;
    
    bool anchored = false;
    rp::RigidBody* rigidBody = nullptr;

    void syncTransforms();
};