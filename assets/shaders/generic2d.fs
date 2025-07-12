#version 330 core

// I/O

out vec4 fColor;
uniform vec4 aColor;

// Main

void main() {
    fColor = aColor;
}