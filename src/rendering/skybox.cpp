#include <GL/glew.h>
#include <GL/gl.h>
#include <stb/stb_image.h>

#include "skybox.h"

Skybox::Skybox(std::array<std::string, 6> faces, unsigned int format) {
    glGenTextures(1, &this->ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->ID);

    // stbi_set_flip_vertically_on_load(true);
    for (unsigned int i = 0; i< faces.size(); i++) {
        int width, height, nrChannels;
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height,
                                        &nrChannels, 0);

        if (!data) {
            printf("Failed to load texture '%s'\n", faces[i].c_str());
            abort();
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format,
                    GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    // Wrapping
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Interpolation
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Skybox::~Skybox() {
    glDeleteTextures(1, &this->ID);
}

void Skybox::activate(unsigned int textureIdx) {
    glActiveTexture(GL_TEXTURE0 + textureIdx);
    glBindTexture(GL_TEXTURE_2D, this->ID);
}
