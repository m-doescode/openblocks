#pragma once
#include <glm/glm.hpp>
#include "material.h"

struct Part {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    Material material;
};