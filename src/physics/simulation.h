#pragma once

#include "../part.h"

void simulationInit();
void syncPartPhysics(Part& part);
void physicsStep(float deltaTime);