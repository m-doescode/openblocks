#pragma once
#include <string>

class Shader {
    unsigned int id;

public:
    void use();
    Shader(std::string vertexShaderPath, std::string fragmentShaderPath);
    ~Shader();
};