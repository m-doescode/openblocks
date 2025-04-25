#pragma once

#include "base.h"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <reactphysics3d/reactphysics3d.h>

namespace rp = reactphysics3d;

namespace Data {
    class Color3 : Base {
        float r;
        float g;
        float b;
    
    public:
        Color3(float r, float g, float b);
        Color3(const glm::vec3&);
        ~Color3();

        static Color3 FromHex(std::string hex);

        virtual const TypeInfo& GetType() const override;
        static const TypeInfo TYPE;

        virtual const Data::String ToString() const override;
        std::string ToHex() const;
        virtual void Serialize(pugi::xml_node node) const override;
        virtual void PushLuaValue(lua_State*) const override;
        static Data::Variant Deserialize(pugi::xml_node node);

        operator glm::vec3() const;

        inline float R() const { return r; }
        inline float G() const { return g; }
        inline float B() const { return b; }
    };
}

using Data::Color3;