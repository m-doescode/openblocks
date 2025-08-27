#pragma once
#include "datatypes/cframe.h"
#include "datatypes/vector.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext.hpp>

#include <Jolt/Jolt.h>

template <typename T, typename F>
T convert(F vec) = delete;

// Vector3

template <>
inline Vector3 convert<Vector3>(JPH::Vec3 vec) {
    return Vector3(vec.GetX(), vec.GetY(), vec.GetZ());
}

template <>
inline JPH::Vec3 convert<JPH::Vec3>(Vector3 vec) {
    return JPH::Vec3(vec.X(), vec.Y(), vec.Z());
}

template <>
inline glm::vec3 convert<glm::vec3>(JPH::Vec3 vec) {
    return glm::vec3(vec.GetX(), vec.GetY(), vec.GetZ());
}

template <>
inline JPH::Vec3 convert<JPH::Vec3>(glm::vec3 vec) {
    return JPH::Vec3(vec.x, vec.y, vec.z);
}

// Quaternion

template <>
inline glm::quat convert<glm::quat>(JPH::Quat quat) {
    return glm::quat(quat.GetW(), quat.GetX(), quat.GetY(), quat.GetZ());
}

template <>
inline JPH::Quat convert<JPH::Quat>(glm::quat quat) {
    return JPH::Quat(quat.x, quat.y, quat.z, quat.w);
}