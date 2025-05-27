#version 330 core

uniform vec3 scale;

in vec3 vPos;

out vec4 fColor;
uniform vec3 color;

void main() {
    // float thickness = 0.2;

    // vec3 distanceFromEdge = abs(scale * vPos) - scale / 2;

    // vec3 vec = distanceFromEdge + vec3(thickness / 2);
    // vec3 signs = max(sign(vec), 0);
    // float negatives = signs.x + signs.y + signs.z;

    // if (negatives >= 2)
    //     fColor = vec4(0.204, 0.584, 0.922, 1);
    // else
    //     fColor = vec4(0);

    // fColor = vec4(0.204, 0.584, 0.922, 1);
    fColor = vec4(color, 1);
}