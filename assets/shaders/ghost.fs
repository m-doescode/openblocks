#version 330 core

in vec3 vPos;
in vec3 vNormal;

out vec4 FragColor;

#define NR_POINT_LIGHTS 4

uniform vec3 viewPos;
uniform float transparency;
uniform vec3 color;



// Main

void main() {
    FragColor = vec4(color, 1) * (1-transparency);
}