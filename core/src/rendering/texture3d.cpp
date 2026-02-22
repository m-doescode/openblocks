#include "texture3d.h"

#include <glad/gl.h>
#include <stb_image.h>

#include "panic.h"
#include "logger.h"
#include "rendering/assets.h"

Texture3D::Texture3D(const char* texturePath, unsigned int tileWidth, unsigned int tileHeight, unsigned int tileCount, unsigned int format) {
    glGenTextures(1, &this->ID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->ID);

    // Wrapping
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);

    // Interpolation
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(resolveAssetPath(texturePath).c_str(), &width, &height,
                                    &nrChannels, 0);

    if (!data) {
        Logger::fatalErrorf("Failed to load texture '%s'", texturePath);
        panic();
    }

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, tileWidth, tileHeight, /* no of layers= */ tileCount, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    stbi_image_free(data);
}

Texture3D::~Texture3D() {
    glDeleteTextures(1, &this->ID);
}

void Texture3D::activate(unsigned int textureIdx) {
    glActiveTexture(GL_TEXTURE0 + textureIdx);
    glBindTexture(GL_TEXTURE_2D, this->ID);
}
