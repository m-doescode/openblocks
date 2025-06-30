#version 330 core

out vec4 FragColor;
in vec3 vPos;
in vec2 vTexCoord;

uniform sampler2D fontTex;
uniform int charIndex;

// Main

void main() {
    int x = (charIndex-32) % 16;
    int y = (charIndex-32) / 16;

    float fx = float(x) / 16;
    float fy = float(y) / 8;

   vec4 color = texture(fontTex, vec2(fx, fy) + vTexCoord * vec2(1.f/32, 1.f/16));
   FragColor = vec3(color) == vec3(0, 0, 0) ? vec4(0, 0, 0, 0) : color;
//    FragColor = color;
}