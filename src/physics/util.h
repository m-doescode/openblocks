#pragma once
#include <reactphysics3d/mathematics/Quaternion.h>
#include <reactphysics3d/mathematics/Vector3.h>
#include <reactphysics3d/mathematics/mathematics.h>
#include <glm/ext.hpp>

namespace rp = reactphysics3d;

inline rp::Vector3 glmToRp(glm::vec3 vec) {
    return rp::Vector3(vec.x, vec.y, vec.z);
}

inline rp::Quaternion glmToRp(glm::quat quat) {
    return rp::Quaternion(rp::Vector3(quat.x, quat.y, quat.z), quat.w);
}