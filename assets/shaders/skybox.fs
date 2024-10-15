#version 330 core

uniform samplerCube skybox;

// in vec3 vPos;
// in vec3 vNormal;
in vec3 vTexCoords;

out vec4 fColor;

// Main

void main() {
    // fColor = texture(uTexture, vTexCoords);
    fColor = texture(skybox, vTexCoords);
}
