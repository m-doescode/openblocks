#pragma once
#include <GLFW/glfw3.h>

extern bool wireframeRendering;

namespace Data { class CFrame; };

void renderInit(GLFWwindow* window, int width, int height);
void render(GLFWwindow* window);
void setViewport(int width, int height);
void addDebugRenderCFrame(Data::CFrame);