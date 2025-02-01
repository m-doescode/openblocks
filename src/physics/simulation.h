#pragma once

#include "../objects/part.h"
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <reactphysics3d/collision/RaycastInfo.h>

void simulationInit();
void syncPartPhysics(std::shared_ptr<Part> part);
void physicsStep(float deltaTime);
void castRay(glm::vec3 point, glm::vec3 rotation, float maxLength, rp::RaycastCallback* callback);