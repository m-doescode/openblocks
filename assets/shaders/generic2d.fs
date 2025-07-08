#version 330 core

// I/O

out vec4 fColor;
uniform vec3 aColor;

// Main

void main() {
    fColor = vec4(aColor, 1.0);
}