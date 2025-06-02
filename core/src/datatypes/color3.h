#pragma once

#include "base.h"
#include "datatypes/annotation.h"
#include <glm/ext/vector_float3.hpp>

class DEF_DATA Color3 {
    AUTOGEN_PREAMBLE_DATA

    float r;
    float g;
    float b;

public:
    DEF_DATA_CTOR Color3(float r, float g, float b);
    Color3(const glm::vec3&);
    ~Color3();

    DEF_DATA_METHOD static Color3 FromHex(std::string hex);

    virtual const std::string ToString() const;
    DEF_DATA_METHOD std::string ToHex() const;
    virtual void Serialize(pugi::xml_node node) const;
    static Color3 Deserialize(pugi::xml_node node);

    static void PushLuaLibrary(lua_State*);

    operator glm::vec3() const;

    DEF_DATA_PROP inline float R() const { return r; }
    DEF_DATA_PROP inline float G() const { return g; }
    DEF_DATA_PROP inline float B() const { return b; }
};