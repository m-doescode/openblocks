#include "vector.h"
#include <cstdio>
#include <cstdlib>
#include <glm/ext/quaternion_geometric.hpp>
#include <string>
#include "datatypes/base.h"
#include "meta.h" // IWYU pragma: keep

Data::Vector3::Vector3() : vector(glm::vec3(0, 0, 0)) {};
Data::Vector3::Vector3(const glm::vec3& src) : vector(src) {};
Data::Vector3::Vector3(const rp::Vector3& src) : vector(glm::vec3(src.x, src.y, src.z)) {};
Data::Vector3::Vector3(float x, const float y, float z) : vector(glm::vec3(x, y, z)) {};

Data::Vector3::~Vector3() = default;
const Data::TypeInfo Data::Vector3::TYPE = {
    .name = "Vector3",
    .deserializer = &Data::Vector3::Deserialize,
    .fromString = &Data::Vector3::FromString,
};

const Data::TypeInfo& Data::Vector3::GetType() const { return Data::Vector3::TYPE; };

Data::Vector3 Data::Vector3::ZERO(0, 0, 0);
Data::Vector3 Data::Vector3::ONE(1, 1, 1);

const Data::String Data::Vector3::ToString() const {
    // https://stackoverflow.com/a/46424921/16255372
    std::stringstream stream;
    stream << std::setprecision(8) << std::noshowpoint << X() << ", " << Y() << ", " << Z();
    return stream.str();
}

Data::Vector3::operator glm::vec3() const { return vector; };
Data::Vector3::operator rp::Vector3() const { return rp::Vector3(X(), Y(), Z()); };

// Operators

Data::Vector3 Data::Vector3::operator *(float scale) const {
    return Data::Vector3(this->X() * scale, this->Y() * scale, this->Z() * scale);
}

Data::Vector3 Data::Vector3::operator /(float scale) const {
    return Data::Vector3(this->X() / scale, this->Y() / scale, this->Z() / scale);
}

// Component-wise
Data::Vector3 Data::Vector3::operator *(Data::Vector3 other) const {
    return Data::Vector3(this->X() * other.X(), this->Y() * other.Y(), this->Z() * other.Z());
}

Data::Vector3 Data::Vector3::operator +(Data::Vector3 other) const {
    return Data::Vector3(this->X() + other.X(), this->Y() + other.Y(), this->Z() + other.Z());
}

Data::Vector3 Data::Vector3::operator -(Data::Vector3 other) const {
    return Data::Vector3(this->X() - other.X(), this->Y() - other.Y(), this->Z() - other.Z());
}

Data::Vector3 Data::Vector3::operator -() const {
    return Data::Vector3(-this->X(), -this->Y(), -this->Z());
}

bool Data::Vector3::operator ==(Data::Vector3 other) const {
    return this->X() == other.X() && this->Y() == other.Y() && this->Z() == other.Z();
}

bool Data::Vector3::operator <(Data::Vector3 other) const {
    return X() < other.X() && Y() < other.Y() && Z() < other.Z();
}

bool Data::Vector3::operator >(Data::Vector3 other) const {
    return X() > other.X() && Y() > other.Y() && Z() > other.Z();
}

Data::Vector3 Data::Vector3::Cross(Data::Vector3 other) const {
    return glm::cross(this->vector, other.vector);
}

float Data::Vector3::Dot(Data::Vector3 other) const {
    return glm::dot(this->vector, other.vector);
}

// Serialization

void Data::Vector3::Serialize(pugi::xml_node node) const {
    node.append_child("X").text().set(std::to_string(this->X()));
    node.append_child("Y").text().set(std::to_string(this->Y()));
    node.append_child("Z").text().set(std::to_string(this->Z()));
}

Data::Variant Data::Vector3::Deserialize(pugi::xml_node node) {
    return Data::Vector3(node.child("X").text().as_float(), node.child("Y").text().as_float(), node.child("Z").text().as_float());
}

std::optional<Data::Variant> Data::Vector3::FromString(std::string string) {
    float components[3];

    for (int i = 0; i < 3; i++) {
        if (string.length() == 0) return std::nullopt;
        while (string[0] == ' ' && string.length() > 0) string.erase(0, 1);
        size_t nextPos = string.find(",");
        if (nextPos == -1) nextPos = string.length();
        std::string term = string.substr(0, nextPos);
        string.erase(0, nextPos+1);

        char* cpos;
        float value = std::strtof(term.c_str(), &cpos);
        if (cpos == term.c_str()) return std::nullopt;

        components[i] = value;
    }

    return Data::Vector3(components[0], components[1], components[2]);
}

// Lua (TEMPORARY)
#include "lua.h"

static int lib_index(lua_State*);
static const struct luaL_Reg lib_metatable [] = {
    {"__index", lib_index},
    {NULL, NULL} /* end of array */
};

static int lib_index(lua_State* L) {
    std::string key(lua_tostring(L, 2));
    lua_pop(L, 2);

    if (key == "test") {
        Data::String("mabaref").PushLuaValue(L);
        return 1;
    }

    return luaL_error(L, "%s is not a valid member of %s\n", key.c_str(), "Vector3");
}

static int data_index(lua_State*);
static const struct luaL_Reg metatable [] = {
    {"__index", data_index},
    {NULL, NULL} /* end of array */
};

static int data_index(lua_State* L) {
    auto this__ = (Data::Base*)lua_touserdata(L, 1);
    this__->GetType();
    if (&this__->GetType() != &Vector3::TYPE) return luaL_typerror(L, 0, "Vector3");
    Vector3* this_ = (Vector3*)this__;

    std::string key(lua_tostring(L, 2));
    lua_pop(L, 2);

    if (key == "X") {
        Data::Float(this_->X()).PushLuaValue(L);
        return 1;
    } else if (key == "Magnitude") {
        lua_pushcfunction(L, [](lua_State* L) {
            auto this__ = (Data::Base*)lua_touserdata(L, 1);
            if (&this__->GetType() != &Vector3::TYPE) return luaL_typerror(L, 0, "Vector3");
            Vector3* this_ = (Vector3*)this__;

            Data::Float(this_->Magnitude()).PushLuaValue(L);
            return 1;
        });
        return 1;
    } else if (key == "Dot") {
        lua_pushcfunction(L, [](lua_State* L) {
            auto this__ = (Data::Base*)lua_touserdata(L, 1);
            if (&this__->GetType() != &Vector3::TYPE) return luaL_typerror(L, 0, "Vector3");
            Vector3* this_ = (Vector3*)this__;

            auto arg0_ = (Data::Base*)lua_touserdata(L, 2);
            if (&arg0_->GetType() != &Vector3::TYPE) return luaL_typerror(L, 1, "Vector3");
            Vector3* arg0 = (Vector3*)arg0_;

            Data::Float(this_->Dot(*arg0)).PushLuaValue(L);
            return 1;
        });
        return 1;
    }

    return luaL_error(L, "%s is not a valid member of %s\n", key.c_str(), "Vector3");
}

void Data::Vector3::PushLuaLibrary(lua_State* L) {
    int n = lua_gettop(L);

    lua_newuserdata(L, 0);

    // Create the library's metatable
    luaL_newmetatable(L, "__mt_lib_Vector3");
    luaL_register(L, NULL, lib_metatable);

    lua_setmetatable(L, n+1);
}

void Data::Vector3::PushLuaValue(lua_State* L) const {
    int n = lua_gettop(L);

    // I'm torn... should this be Data::Variant, or Data::Base?
    // If I ever decouple typing from Data::Base, I'll switch it to variant,
    // otherwise, it doesn't make much sense to represent it as one
    Vector3* userdata = (Vector3*)lua_newuserdata(L, sizeof(Vector3));
    new(userdata) Vector3(*this);

    // Create the library's metatable
    luaL_newmetatable(L, "__mt_Vector3");
    luaL_register(L, NULL, metatable);

    lua_setmetatable(L, n+1);
}