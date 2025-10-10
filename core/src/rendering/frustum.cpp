#include "frustum.h"
#include "datatypes/vector.h"
#include <glm/ext/matrix_clip_space.hpp>

// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling

// https://stackoverflow.com/q/66227192/16255372
FrustumPlane::FrustumPlane(Vector3 point, Vector3 normal) : normal(normal.Unit()), distance(normal.Unit().Dot(point)) {}

Frustum::Frustum(const Camera cam, float aspect, float fovY, float zNear, float zFar) {
    const float halfVSide = zFar * tanf(fovY * 0.5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * cam.cameraFront;

    // Don't forget to normalize!!!
    glm::vec3 camRight = glm::normalize(glm::cross(cam.cameraFront, -cam.cameraUp)); // Technically this is left, but whatever
    glm::vec3 trueCamUp = glm::cross(cam.cameraFront, camRight);
    near = { cam.cameraPos + zNear * cam.cameraFront, cam.cameraFront };
    far = { cam.cameraPos + frontMultFar, -cam.cameraFront };
    right = { cam.cameraPos,
                            glm::cross(frontMultFar - camRight * halfHSide, trueCamUp) };
    left = { cam.cameraPos,
                            glm::cross(trueCamUp,frontMultFar + camRight * halfHSide) };
    top = { cam.cameraPos,
                            glm::cross(camRight, frontMultFar - trueCamUp * halfVSide) };
    bottom = { cam.cameraPos,
                            glm::cross(frontMultFar + trueCamUp * halfVSide, camRight) };
}

bool FrustumPlane::checkPointForward(Vector3 point) {
    return (normal.Dot(point) - distance) < 0;
}

bool Frustum::checkPoint(Vector3 point) {
    return true
        // TODO: Near and far are broken for some reason
        // && near.checkPointForward(point)
        // && far.checkPointForward(point)
        && left.checkPointForward(point)
        && right.checkPointForward(point)
        && top.checkPointForward(point)
        && bottom.checkPointForward(point)
        ;
}