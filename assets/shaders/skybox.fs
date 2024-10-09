#version 330 core

uniform sampler2D uTexture;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoords;

out vec4 fColor;

// Main

void main() {
    fColor = vec4(vTexCoords, 1.0, 1.0);
}
