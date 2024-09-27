#pragma once
#include <glm/ext/vector_float3.hpp>

struct Material {
    glm::vec3 diffuse;
    glm::vec3 specular; 
    float shininess; 
};