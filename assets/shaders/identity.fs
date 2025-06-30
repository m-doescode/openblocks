#version 330 core

// I/O

out vec4 FragColor;

uniform vec4 aColor;

// Main

void main() {
    FragColor = aColor;
}