#version 330 core

out vec4 FragColor;

uniform vec3 color;

// Main

void main() {
    FragColor = vec4(color, 1);
}