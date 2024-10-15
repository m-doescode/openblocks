#pragma once

#include <array>
#include <string>
#include <vector>
#define GL_RGB 0x1907

class Skybox {
private:
    unsigned int ID;
public:
    Skybox(std::array<std::string, 6>, unsigned format = GL_RGB);
    ~Skybox();

    void activate(unsigned int textureIdx);
};
