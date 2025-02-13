#pragma once

#include "../objects/part.h"
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <reactphysics3d/collision/RaycastInfo.h>

struct RaycastResult {
    rp::Vector3 worldPoint;
    rp::Vector3 worldNormal;
    rp::decimal hitFraction;
    int triangleIndex;
    rp::Body* body;
    rp::Collider* collider;

    RaycastResult(const rp::RaycastInfo& raycastInfo);
};

enum FilterResult {
    TARGET, // The object is captured
    BLOCK, // The object blocks any objects behind it, but is not captured
    PASS, // The object is transparent, ignore it
};

typedef std::function<FilterResult(std::shared_ptr<Part>)> RaycastFilter;

void simulationInit();
void syncPartPhysics(std::shared_ptr<Part> part);
void physicsStep(float deltaTime);
std::optional<const RaycastResult> castRayNearest(glm::vec3 point, glm::vec3 rotation, float maxLength, std::optional<RaycastFilter> filter = std::nullopt, unsigned short categoryMaskBits = 0xFFFF);