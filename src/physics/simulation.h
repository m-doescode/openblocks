#pragma once

#include "../part.h"

void simulationInit();
void addToSimulation(Part part);
void physicsStep(float deltaTime);