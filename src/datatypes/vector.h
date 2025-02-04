#pragma once

#include "base.h"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <reactphysics3d/reactphysics3d.h>

namespace rp = reactphysics3d;

namespace Data {
    class Vector3 : Base {
        const glm::vec3 vector;
    
    public:
        Vector3(float x, float y, float z);
        Vector3(const glm::vec3&);
        Vector3(const rp::Vector3&);
        ~Vector3();

        virtual const TypeInfo& GetType() const override;
        static const TypeInfo TYPE;

        virtual const Data::String ToString() const override;

        operator glm::vec3() const;
        operator rp::Vector3() const;

        inline float X() const { return vector.x; }
        inline float Y() const { return vector.y; }
        inline float Z() const { return vector.z; }
        inline float Magnitude() const { return glm::length(vector); }
    };
}
