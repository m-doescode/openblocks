#pragma once
#include <GLFW/glfw3.h>

extern bool wireframeRendering;

void renderInit(GLFWwindow* window, int width, int height);
void render(GLFWwindow* window);
void setViewport(int width, int height);