#pragma once

#include "camera.h"
#include "datatypes/vector.h"

// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling

struct FrustumPlane {
    Vector3 normal;
    float distance; // leastPoint = normal * distance
                    // leastPoint is the closest point to (0,0)

    FrustumPlane(Vector3 point, Vector3 normal);
    FrustumPlane() = default;

    bool checkPointForward(Vector3);
};

struct Frustum {
    FrustumPlane near;
    FrustumPlane far;
    FrustumPlane left;
    FrustumPlane right;
    FrustumPlane top;
    FrustumPlane bottom;

    Frustum(const Camera cam, float aspect, float fovY, float zNear, float zFar);
    bool checkPoint(Vector3);
};