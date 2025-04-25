#include "base.h"
#include "error/data.h"
#include "meta.h"
#include <ios>
#include <sstream>
#include "lua.h"

#define IMPL_WRAPPER_CLASS(CLASS_NAME, WRAPPED_TYPE, TYPE_NAME) Data::CLASS_NAME::CLASS_NAME(WRAPPED_TYPE in) : value(in) {} \
Data::CLASS_NAME::~CLASS_NAME() = default; \
Data::CLASS_NAME::operator const WRAPPED_TYPE() const { return value; } \
const Data::TypeInfo Data::CLASS_NAME::TYPE = { \
    .name = TYPE_NAME, \
    .deserializer = &Data::CLASS_NAME::Deserialize, \
    .fromString = &Data::CLASS_NAME::FromString, \
    .fromLuaValue = &Data::CLASS_NAME::FromLuaValue, \
}; \
const Data::TypeInfo& Data::CLASS_NAME::GetType() const { return Data::CLASS_NAME::TYPE; }; \
void Data::CLASS_NAME::Serialize(pugi::xml_node node) const { node.text().set(std::string(this->ToString())); }

Data::Base::~Base() {};

Data::Null::Null() {};
Data::Null::~Null() = default;
const Data::TypeInfo Data::Null::TYPE = {
    .name = "null",
    .deserializer = &Data::Null::Deserialize,
};
const Data::TypeInfo& Data::Null::GetType() const { return Data::Null::TYPE; };

const Data::String Data::Null::ToString() const {
    return Data::String("null");
}

void Data::Null::Serialize(pugi::xml_node node) const {
    node.text().set("null");
}

Data::Variant Data::Null::Deserialize(pugi::xml_node node) {
    return Data::Null();
}

void Data::Null::PushLuaValue(lua_State* L) const {
    lua_pushnil(L);    
}

result<Data::Variant, LuaCastError> Data::Null::FromLuaValue(lua_State* L, int idx) {
    return Data::Variant(Data::Null());
}

//

IMPL_WRAPPER_CLASS(Bool, bool, "bool")
IMPL_WRAPPER_CLASS(Int, int, "int")
IMPL_WRAPPER_CLASS(Float, float, "float")
IMPL_WRAPPER_CLASS(String, std::string, "string")

// ToString

const Data::String Data::Bool::ToString() const {
    return Data::String(value ? "true" : "false");
}

const Data::String Data::Int::ToString() const {
    return Data::String(std::to_string(value));
}

const Data::String Data::Float::ToString() const {
    std::stringstream stream;
    stream << std::noshowpoint << value;
    return Data::String(stream.str());
}

const Data::String Data::String::ToString() const {
    return *this;
}

// Deserialize

Data::Variant Data::Bool::Deserialize(pugi::xml_node node) {
    return Data::Bool(node.text().as_bool());
}

Data::Variant Data::Int::Deserialize(pugi::xml_node node) {
    return Data::Int(node.text().as_int());
}

Data::Variant Data::Float::Deserialize(pugi::xml_node node) {
    return Data::Float(node.text().as_float());
}

Data::Variant Data::String::Deserialize(pugi::xml_node node) {
    return Data::String(node.text().as_string());
}

// FromString

std::optional<Data::Variant> Data::Bool::FromString(std::string string) {
    return Data::Bool(string[0] == 't' || string[0] == 'T' || string[0] == '1' || string[0] == 'y' || string[0] == 'Y');
}

std::optional<Data::Variant> Data::Int::FromString(std::string string) {
    char* endPos;
    int value = (int)std::strtol(string.c_str(), &endPos, 10);
    if (endPos == string.c_str()) return std::nullopt;
    return Data::Int(value);
}

std::optional<Data::Variant> Data::Float::FromString(std::string string) {
    char* endPos;
    float value = std::strtof(string.c_str(), &endPos);
    if (endPos == string.c_str()) return std::nullopt;
    return Data::Float(value);
}

std::optional<Data::Variant> Data::String::FromString(std::string string) {
    return Data::String(string);
}

// PushLuaValue

void Data::Bool::PushLuaValue(lua_State* L) const {
    lua_pushboolean(L, *this);    
}

void Data::Int::PushLuaValue(lua_State* L) const {
    lua_pushinteger(L, *this);    
}

void Data::Float::PushLuaValue(lua_State* L) const {
    lua_pushnumber(L, *this);    
}

void Data::String::PushLuaValue(lua_State* L) const {
    lua_pushstring(L, value.c_str());    
}

// FromLuaValue

result<Data::Variant, LuaCastError> Data::Bool::FromLuaValue(lua_State* L, int idx) {
    if (!lua_isboolean(L, idx))
        return LuaCastError(lua_typename(L, idx), "boolean");
    return Data::Variant(Data::Bool(lua_toboolean(L, idx)));
}

result<Data::Variant, LuaCastError> Data::Int::FromLuaValue(lua_State* L, int idx) {
    if (!lua_isnumber(L, idx))
        return LuaCastError(lua_typename(L, idx), "integer");
    return Data::Variant(Data::Int((int)lua_tonumber(L, idx)));
}

result<Data::Variant, LuaCastError> Data::Float::FromLuaValue(lua_State* L, int idx) {
    if (!lua_isnumber(L, idx))
        return LuaCastError(lua_typename(L, idx), "float");
    return Data::Variant(Data::Float((float)lua_tonumber(L, idx)));
}

result<Data::Variant, LuaCastError> Data::String::FromLuaValue(lua_State* L, int idx) {
    if (!lua_tostring(L, idx))
        return LuaCastError(lua_typename(L, idx), "string");
    return Data::Variant(Data::String(lua_tostring(L, idx)));
}