#pragma once

#include "base.h"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <reactphysics3d/reactphysics3d.h>

namespace rp = reactphysics3d;

namespace Data {
    class Vector3 : Base {
        glm::vec3 vector;
    
    public:
        Vector3(float x, float y, float z);
        Vector3(const glm::vec3&);
        Vector3(const rp::Vector3&);
        ~Vector3();

        virtual const TypeInfo& GetType() const override;
        static const TypeInfo TYPE;

        static Data::Vector3 ZERO;
        static Data::Vector3 ONE;

        virtual const Data::String ToString() const override;
        virtual void Serialize(pugi::xml_node* node) const override;
        static Data::Variant Deserialize(pugi::xml_node* node);

        operator glm::vec3() const;
        operator rp::Vector3() const;

        inline float X() const { return vector.x; }
        inline float Y() const { return vector.y; }
        inline float Z() const { return vector.z; }
        inline float Magnitude() const { return glm::length(vector); }
        inline Data::Vector3 Unit() const { return glm::normalize(vector); }
        inline Data::Vector3 Abs() const { return glm::abs(vector); }

        Data::Vector3 Cross(Data::Vector3) const;
        float Dot(Data::Vector3) const;
    
        // Operators
        Data::Vector3 operator *(float) const;
        Data::Vector3 operator /(float) const;
        Data::Vector3 operator *(Data::Vector3) const; // Component-wise
        Data::Vector3 operator +(Data::Vector3) const;
        Data::Vector3 operator -(Data::Vector3) const;
        Data::Vector3 operator -() const;
    };
}
