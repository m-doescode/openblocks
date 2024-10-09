#pragma once

#define GL_RGB 0x1907

class Texture {
private:
    unsigned int ID;
public:
    Texture(const char* texturePath, unsigned format = GL_RGB);
    ~Texture();

    void activate(unsigned int textureIdx);
};
