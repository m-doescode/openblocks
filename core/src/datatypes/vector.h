#pragma once

#include "base.h"
#include "datatypes/annotation.h"
#include "error/data.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <reactphysics3d/mathematics/Vector3.h>

// namespace reactphysics3d { class Vector3; };

class DEF_DATA Vector3 {
    AUTOGEN_PREAMBLE_DATA
    glm::vec3 vector;

public:
    DEF_DATA_CTOR Vector3();
    DEF_DATA_CTOR Vector3(float x, float y, float z);
    inline Vector3(float value) : Vector3(value, value, value) {}
    Vector3(const glm::vec3&);
    Vector3(const reactphysics3d::Vector3&);
    virtual ~Vector3();

    DEF_DATA_PROP static Vector3 ZERO;
    DEF_DATA_PROP static Vector3 ONE;

    virtual const std::string ToString() const;
    virtual void Serialize(pugi::xml_node node) const;

    static result<Vector3, DataParseError> Deserialize(pugi::xml_node node);
    static result<Vector3, DataParseError> FromString(std::string);
    
    static void PushLuaLibrary(lua_State*);

    operator glm::vec3() const;
    operator reactphysics3d::Vector3() const;

    DEF_DATA_PROP inline float X() const { return vector.x; }
    DEF_DATA_PROP inline float Y() const { return vector.y; }
    DEF_DATA_PROP inline float Z() const { return vector.z; }
    DEF_DATA_METHOD inline float Magnitude() const { return glm::length(vector); }
    DEF_DATA_METHOD inline Vector3 Unit() const { return glm::normalize(vector); }
    DEF_DATA_METHOD inline Vector3 Abs() const { return glm::abs(vector); }

    DEF_DATA_METHOD Vector3 Cross(Vector3) const;
    DEF_DATA_METHOD float Dot(Vector3) const;

    // Operators
    DEF_DATA_OP Vector3 operator *(float) const;
    DEF_DATA_OP Vector3 operator /(float) const;
    DEF_DATA_OP Vector3 operator *(Vector3) const; // Component-wise
    DEF_DATA_OP Vector3 operator /(Vector3) const; // Component-wise
    DEF_DATA_OP Vector3 operator +(Vector3) const;
    DEF_DATA_OP Vector3 operator -(Vector3) const;
    DEF_DATA_OP Vector3 operator -() const;

    DEF_DATA_OP bool operator <(Vector3) const;
    DEF_DATA_OP bool operator >(Vector3) const;

    DEF_DATA_OP bool operator ==(Vector3) const;

    // Augmented shorthands
    inline Vector3 operator *=(float factor) const { return *this * factor; }
    inline Vector3 operator /=(float factor) const { return *this / factor; }
    inline Vector3 operator *=(Vector3 factor) const { return *this * factor; }
    inline Vector3 operator /=(Vector3 factor) const { return *this / factor; }
    inline Vector3 operator +=(Vector3 vector) const { return *this + vector; }
    inline Vector3 operator -=(Vector3 vector) const { return *this + vector; }
};

inline void printVec(Vector3 vec) {
    printf("(%f, %f, %f)\n", vec.X(), vec.Y(), vec.Z());
}