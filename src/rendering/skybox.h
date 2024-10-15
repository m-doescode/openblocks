#pragma once

#include <string>
#include <vector>
#define GL_RGB 0x1907

class Skybox {
private:
    unsigned int ID;
public:
    Skybox(std::vector<std::string>, unsigned format = GL_RGB);
    ~Skybox();

    void activate(unsigned int textureIdx);
};
