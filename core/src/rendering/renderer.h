#pragma once
#include <GLFW/glfw3.h>

void renderInit(GLFWwindow* window, int width, int height);
void render(GLFWwindow* window);
void setViewport(int width, int height);