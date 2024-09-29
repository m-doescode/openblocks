#pragma once
#include <glm/ext/vector_float3.hpp>
#include <reactphysics3d/mathematics/Quaternion.h>
#include <reactphysics3d/mathematics/Vector3.h>
#include <reactphysics3d/mathematics/mathematics.h>
#include <glm/ext.hpp>

namespace rp = reactphysics3d;

inline rp::Vector3 glmToRp(glm::vec3 vec) {
    return rp::Vector3(vec.x, vec.y, vec.z);
}

inline rp::Quaternion glmToRp(glm::quat quat) {
    return rp::Quaternion(quat.w, rp::Vector3(quat.x, quat.y, quat.z));
}

inline glm::vec3 rpToGlm(rp::Vector3 vec) {
    return glm::vec3(vec.x, vec.y, vec.z);
}

inline glm::quat rpToGlm(rp::Quaternion quat) {
    return glm::quat(quat.w, quat.x, quat.y, quat.z);
}