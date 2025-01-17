#pragma once

#include "../objects/part.h"
#include <memory>

void simulationInit();
void syncPartPhysics(std::shared_ptr<Part> part);
void physicsStep(float deltaTime);