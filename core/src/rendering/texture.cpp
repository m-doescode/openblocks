#include "texture.h"

#include <glad/gl.h>
#include <stb_image.h>

#include "panic.h"
#include "logger.h"
#include "rendering/assets.h"

Texture::Texture(const char* texturePath, unsigned int format, bool noMipMaps) {
    glGenTextures(1, &this->ID);
    glBindTexture(GL_TEXTURE_2D, this->ID);

    // Wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Interpolation
    if (!noMipMaps) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (noMipMaps) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(resolveAssetPath(texturePath).c_str(), &width, &height,
                                    &nrChannels, 0);

    if (!data) {
        Logger::fatalErrorf("Failed to load texture '%s'", texturePath);
        panic();
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    if (!noMipMaps) glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

Texture::~Texture() {
    glDeleteTextures(1, &this->ID);
}

void Texture::activate(unsigned int textureIdx) {
    glActiveTexture(GL_TEXTURE0 + textureIdx);
    glBindTexture(GL_TEXTURE_2D, this->ID);
}
