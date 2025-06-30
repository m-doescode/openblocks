#pragma once
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <string>
#include "material.h"
#include "light.h"

class Shader {
    unsigned int id;

public:
    void use();
    Shader(std::string vertexShaderPath, std::string fragmentShaderPath);
    ~Shader();

    void set(std::string key, int value);
    void set(std::string key, float value);
    void set(std::string key, Material value);
    void set(std::string key, DirLight value);
    void set(std::string key, PointLight value);
    void set(std::string key, glm::vec3 value);
    void set(std::string key, glm::vec4 value);
    void set(std::string key, glm::mat3 value);
    void set(std::string key, glm::mat4 value);

    int getAttribute(std::string key);
};