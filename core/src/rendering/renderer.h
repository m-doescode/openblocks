#pragma once
#include <GLFW/glfw3.h>

extern bool wireframeRendering;

namespace Data { class CFrame; class Color3; };

void renderInit(GLFWwindow* window, int width, int height);
void render(GLFWwindow* window);
void setViewport(int width, int height);
void addDebugRenderCFrame(CFrame);
void addDebugRenderCFrame(CFrame, Color3);