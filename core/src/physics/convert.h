#pragma once
#include "datatypes/cframe.h"
#include "datatypes/vector.h"
#include <glm/ext/vector_float3.hpp>
#include <reactphysics3d/body/Body.h>
#include <reactphysics3d/mathematics/Quaternion.h>
#include <reactphysics3d/mathematics/Vector3.h>
#include <glm/ext.hpp>

#define rp reactphysics3d

template <typename T, typename F>
T convert(F vec) = delete;

// Vector3

template <>
inline Vector3 convert<Vector3>(rp::Vector3 vec) {
    return Vector3(vec.x, vec.y, vec.z);
}

template <>
inline rp::Vector3 convert<rp::Vector3>(Vector3 vec) {
    return rp::Vector3(vec.X(), vec.Y(), vec.Z());
}

template <>
inline glm::vec3 convert<glm::vec3>(rp::Vector3 vec) {
    return glm::vec3(vec.x, vec.y, vec.z);
}

template <>
inline rp::Vector3 convert<rp::Vector3>(glm::vec3 vec) {
    return rp::Vector3(vec.x, vec.y, vec.z);
}

// Quaternion

template <>
inline glm::quat convert<glm::quat>(rp::Quaternion quat) {
    return glm::quat(quat.w, quat.x, quat.y, quat.z);
}

template <>
inline rp::Quaternion convert<rp::Quaternion>(glm::quat quat) {
    return rp::Quaternion(quat.w, rp::Vector3(quat.x, quat.y, quat.z));
}

// CFrame

template <>
inline rp::Transform convert<rp::Transform>(CFrame frame) {
    return rp::Transform(convert<rp::Vector3>(frame.Position()), convert<rp::Quaternion>((glm::quat)frame.RotMatrix()));
}

template <>
inline CFrame convert<CFrame>(rp::Transform trans) {
    return CFrame(convert<Vector3>(trans.getPosition()), convert<glm::quat>(trans.getOrientation()));
}

#undef rp