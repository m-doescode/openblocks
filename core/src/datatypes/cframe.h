#pragma once

#include "base.h"
#include "datatypes/vector.h"
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/matrix.hpp>
#include <reactphysics3d/mathematics/Transform.h>
#include <reactphysics3d/reactphysics3d.h>

namespace rp = reactphysics3d;

namespace Data {
    class CFrame : Base {
        glm::vec3 translation;
        glm::mat3 rotation;
    
        CFrame(glm::vec3, glm::mat3);
    public:
        // CFrame(float x, float y, float z);
        // CFrame(const glm::vec3&);
        // CFrame(const rp::Vector3&);
        CFrame();
        CFrame(float x, float y, float z, float R00, float R01, float R02, float R10, float R11, float R12, float R20, float R21, float R22);
        CFrame(const rp::Transform&);
        CFrame(Data::Vector3 position, glm::quat quat);
        CFrame(Data::Vector3 position, Data::Vector3 lookAt, Data::Vector3 up = Data::Vector3(0, 1, 0));
        ~CFrame();

        static const CFrame IDENTITY;
        static const CFrame YToZ;

        virtual const TypeInfo& GetType() const override;
        static const TypeInfo TYPE;

        virtual const Data::String ToString() const override;
        virtual void Serialize(pugi::xml_node parent) const override;
        static Data::Variant Deserialize(pugi::xml_node node);

        operator glm::mat4() const;
        operator rp::Transform() const;

        //inline static CFrame identity() { }
        inline Vector3 Position() const { return translation; }
        inline CFrame Rotation() const { return CFrame { glm::vec3(0, 0, 0), rotation }; }
        CFrame Inverse() const;
        inline float X() const { return translation.x; }
        inline float Y() const { return translation.y; }
        inline float Z() const { return translation.z; }

        inline Vector3 RightVector() { return glm::column(rotation, 0); }
        inline Vector3 UpVector() { return glm::column(rotation, 1); }
        inline Vector3 LookVector() { return -glm::column(rotation, 2); }

        Vector3 ToEulerAnglesXYZ();
        static CFrame FromEulerAnglesXYZ(Data::Vector3);

        // Operators
        Data::CFrame operator *(Data::CFrame) const;
        Data::Vector3 operator *(Data::Vector3) const;
        Data::CFrame operator +(Data::Vector3) const;
        Data::CFrame operator -(Data::Vector3) const;
    };
}
