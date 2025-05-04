#pragma once

#include "base.h"
#include "datatypes/annotation.h"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <reactphysics3d/reactphysics3d.h>

namespace rp = reactphysics3d;

namespace Data {
    class DEF_DATA Vector3 : public Base {
        glm::vec3 vector;
    
    public:
        DEF_DATA_CTOR Vector3();
        DEF_DATA_CTOR Vector3(float x, float y, float z);
        DEF_DATA_CTOR Vector3(const glm::vec3&);
        DEF_DATA_CTOR Vector3(const rp::Vector3&);
        ~Vector3();

        virtual const TypeInfo& GetType() const override;
        static const TypeInfo TYPE;

        DEF_DATA_PROP static Data::Vector3 ZERO;
        DEF_DATA_PROP static Data::Vector3 ONE;

        virtual const Data::String ToString() const override;
        virtual void Serialize(pugi::xml_node node) const override;

        static Data::Variant Deserialize(pugi::xml_node node);
        static std::optional<Data::Variant> FromString(std::string);
        
        static result<Data::Variant, LuaCastError> FromLuaValue(lua_State*, int idx);
        virtual void PushLuaValue(lua_State*) const override;
        static void PushLuaLibrary(lua_State*);

        operator glm::vec3() const;
        operator rp::Vector3() const;

        DEF_DATA_PROP inline float X() const { return vector.x; }
        DEF_DATA_PROP inline float Y() const { return vector.y; }
        DEF_DATA_PROP inline float Z() const { return vector.z; }
        DEF_DATA_METHOD inline float Magnitude() const { return glm::length(vector); }
        DEF_DATA_METHOD inline Data::Vector3 Unit() const { return glm::normalize(vector); }
        DEF_DATA_METHOD inline Data::Vector3 Abs() const { return glm::abs(vector); }

        DEF_DATA_METHOD Data::Vector3 Cross(Data::Vector3) const;
        DEF_DATA_METHOD float Dot(Data::Vector3) const;
    
        // Operators
        DEF_DATA_OP Data::Vector3 operator *(float) const;
        DEF_DATA_OP Data::Vector3 operator /(float) const;
        DEF_DATA_OP Data::Vector3 operator *(Data::Vector3) const; // Component-wise
        DEF_DATA_OP Data::Vector3 operator +(Data::Vector3) const;
        DEF_DATA_OP Data::Vector3 operator -(Data::Vector3) const;
        DEF_DATA_OP Data::Vector3 operator -() const;

        DEF_DATA_OP bool operator <(Data::Vector3) const;
        DEF_DATA_OP bool operator >(Data::Vector3) const;

        DEF_DATA_OP bool operator ==(Data::Vector3) const;
    };
}

using Data::Vector3;

inline void printVec(Data::Vector3 vec) {
    printf("(%f, %f, %f)\n", vec.X(), vec.Y(), vec.Z());
}