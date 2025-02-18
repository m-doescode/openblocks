#version 330 core

// I/O

out vec4 FragColor;

uniform vec3 aColor;

// Main

void main() {
    FragColor = vec4(aColor, 1);
}