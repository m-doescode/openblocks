#pragma once

extern bool wireframeRendering;

class CFrame;
class Color3;

void renderInit(int width, int height);
void render();
void setViewport(int width, int height);
void addDebugRenderCFrame(CFrame);
void addDebugRenderCFrame(CFrame, Color3);
void setDebugRendererEnabled(bool enabled);