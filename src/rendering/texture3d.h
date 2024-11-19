#pragma once

#define GL_RGB 0x1907

class Texture3D {
private:
    unsigned int ID;
public:
    Texture3D(const char* texturePath, unsigned int tileWidth, unsigned int tileHeight, unsigned int tileCount, unsigned format = GL_RGB);
    ~Texture3D();

    void activate(unsigned int textureIdx);
};
