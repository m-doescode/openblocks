#pragma once

#include "base.h"
#include "datatypes/annotation.h"
#include "datatypes/vector.h"
#include "error/data.h"
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/matrix.hpp>

class DEF_DATA_(name="CoordinateFrame") CFrame {
    AUTOGEN_PREAMBLE_DATA

    glm::vec3 translation;
    glm::mat3 rotation;

    CFrame(glm::vec3, glm::mat3);
public:
    // CFrame(float x, float y, float z);
    // CFrame(const glm::vec3&);
    // CFrame(const rp::Vector3&);
    DEF_DATA_CTOR CFrame();
    DEF_DATA_CTOR CFrame(float x, float y, float z, float R00, float R01, float R02, float R10, float R11, float R12, float R20, float R21, float R22);
    DEF_DATA_CTOR CFrame(Vector3 , Vector3 lookAt, Vector3 up = Vector3(0, 1, 0));
    CFrame(Vector3 position, glm::quat quat);
    virtual ~CFrame();

    // Same as CFrame(position, position + toward), but makes sure that up and toward are not linearly dependant
    static CFrame pointToward(Vector3 position, Vector3 toward);
    // Creates a cframe looking at position + toward, whilst aligning its up to up.
    // If up and toward are approximately linearly dependent (their absolute dot product > 0.999),
    // then the right is used instead
    // Up and right must NOT be linearly dependent
    static CFrame pointAligned(Vector3 position, Vector3 toward, Vector3 up, Vector3 right);

    DEF_DATA_PROP static const CFrame IDENTITY;
    static const CFrame YToZ;

    virtual const std::string ToString() const;
    virtual void Serialize(pugi::xml_node parent) const;
    static result<CFrame, DataParseError> Deserialize(pugi::xml_node node);

    static void PushLuaLibrary(lua_State*);

    operator glm::mat4() const;

    //inline static CFrame identity() { }
    DEF_DATA_PROP inline Vector3 Position() const { return translation; }
    DEF_DATA_PROP inline CFrame Rotation() const { return CFrame { glm::vec3(0, 0, 0), rotation }; }
    inline glm::mat3 RotMatrix() const { return rotation; }
    DEF_DATA_METHOD CFrame Inverse() const;
    DEF_DATA_PROP inline float X() const { return translation.x; }
    DEF_DATA_PROP inline float Y() const { return translation.y; }
    DEF_DATA_PROP inline float Z() const { return translation.z; }

    DEF_DATA_PROP inline Vector3 RightVector() { return glm::column(rotation, 0); }
    DEF_DATA_PROP inline Vector3 UpVector() { return glm::column(rotation, 1); }
    DEF_DATA_PROP inline Vector3 LookVector() { return -glm::column(rotation, 2); }

    DEF_DATA_METHOD Vector3 ToEulerAnglesXYZ();
    DEF_DATA_METHOD static CFrame FromEulerAnglesXYZ(Vector3);

    // Operators
    DEF_DATA_OP CFrame operator *(CFrame) const;
    DEF_DATA_OP Vector3 operator *(Vector3) const;
    DEF_DATA_OP CFrame operator +(Vector3) const;
    DEF_DATA_OP CFrame operator -(Vector3) const;

    inline CFrame operator *=(CFrame otherFrame) { return *this = *this * otherFrame; }
    inline CFrame operator +=(Vector3 offset) { return *this = *this + offset; }
    inline CFrame operator -=(Vector3 offset) { return *this = *this - offset; }

    DEF_DATA_OP bool operator ==(CFrame) const;
};