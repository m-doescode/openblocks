#include "frustum.h"
#include "datatypes/vector.h"
#include <glm/ext/matrix_clip_space.hpp>

// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling

// https://stackoverflow.com/q/66227192/16255372
FrustumPlane::FrustumPlane(Vector3 point, Vector3 normal) : normal(normal.Unit()), distance(normal.Unit().Dot(point)) {}

Frustum::Frustum() {}

Frustum::Frustum(const Camera cam, float aspect, float fovY, float zNear, float zFar) {
    const float halfVSide = zFar * tanf(fovY * 0.5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * -cam.cameraFront;

    // Don't forget to normalize!!!
    glm::vec3 camRight = glm::normalize(glm::cross(cam.cameraFront, cam.cameraUp)); // Technically this is left, but whatever
    glm::vec3 trueCamUp = glm::cross(-cam.cameraFront, camRight);
    near = { cam.cameraPos + zNear * -cam.cameraFront, -cam.cameraFront };
    far = { cam.cameraPos + frontMultFar, cam.cameraFront };
    right = { cam.cameraPos,
                            glm::cross(frontMultFar - camRight * halfHSide, trueCamUp) };
    left = { cam.cameraPos,
                            glm::cross(trueCamUp,frontMultFar + camRight * halfHSide) };
    top = { cam.cameraPos,
                            glm::cross(camRight, frontMultFar - trueCamUp * halfVSide) };
    bottom = { cam.cameraPos,
                            glm::cross(frontMultFar + trueCamUp * halfVSide, camRight) };
}

Frustum Frustum::createSliced(const Camera cam, float width, float height, float left, float right, float top, float bottom, float fovY, float zNear, float zFar) {
    Frustum frustum;

    float aspect = width / height;
    float halfVSide = zFar * tanf(fovY * 0.5f);
    float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * -cam.cameraFront;

    float leftSide = -halfHSide * (left / width * 2 - 1);
    float rightSide = halfHSide * (right / width * 2 - 1);
    float topSide = -halfVSide * (top / height * 2 - 1);
    float bottomSide = halfVSide * (bottom / height * 2 - 1);

    // Don't forget to normalize!!!
    glm::vec3 camRight = glm::normalize(glm::cross(cam.cameraFront, cam.cameraUp)); // Technically this is left, but whatever
    glm::vec3 trueCamUp = glm::cross(-cam.cameraFront, camRight);
    frustum.near = { cam.cameraPos + zNear * -cam.cameraFront, -cam.cameraFront };
    frustum.far = { cam.cameraPos + frontMultFar, cam.cameraFront };
    frustum.right = { cam.cameraPos,
                            glm::cross(frontMultFar - camRight * rightSide, trueCamUp) };
    frustum.left = { cam.cameraPos,
                            glm::cross(trueCamUp,frontMultFar + camRight * leftSide) };
    frustum.top = { cam.cameraPos,
                            glm::cross(camRight, frontMultFar - trueCamUp * topSide) };
    frustum.bottom = { cam.cameraPos,
                            glm::cross(frontMultFar + trueCamUp * bottomSide, camRight) };
    return frustum;
}

bool FrustumPlane::checkPointForward(Vector3 point) {
    return (normal.Dot(point) - distance) > 0;
}

bool FrustumPlane::checkAABBForward(Vector3 center, Vector3 extents) {
    // Not entirely sure how this algorithm works... but hey, when has that ever stopped me?

    // https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    // https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
    // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
    extents = extents * 0.5f;
    const float r = extents.X() * std::abs(normal.X()) +
            extents.Y() * std::abs(normal.Y()) + extents.Z() * std::abs(normal.Z());

    return -r <= (normal.Dot(center) - distance);
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

bool Frustum::checkAABB(Vector3 center, Vector3 extents) {
    return true
        // TODO: Near and far are broken for some reason
        // && near.checkAABBForward(center, extents)
        // && far.checkAABBForward(center, extents)
        && left.checkAABBForward(center, extents)
        && right.checkAABBForward(center, extents)
        && top.checkAABBForward(center, extents)
        && bottom.checkAABBForward(center, extents)
        ;
}