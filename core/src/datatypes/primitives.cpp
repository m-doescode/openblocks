#include "primitives.h"
#include "variant.h"
#include <pugixml.hpp>
#include "lua.h"
#include <sstream>

// null

void Null_Serialize(Variant self, pugi::xml_node node) {
    node.text().set("null");
}

Variant Null_Deserialize(pugi::xml_node node) {
    return std::monostate();
}

const std::string Null_ToString(Variant self) {
    return "null";
}

const std::optional<Variant> Null_FromString(std::string str) {
    return std::monostate();
}

void Null_PushLuaValue(Variant self, lua_State* L) {
    lua_pushnil(L);
}

result<Variant, LuaCastError> Null_FromLuaValue(lua_State* L, int idx) {
    return Variant(std::monostate());
}

const TypeDescriptor NULL_TYPE {
    "null",
    Null_Serialize,
    Null_Deserialize,
    Null_ToString,
    Null_FromString,
    Null_PushLuaValue,
    Null_FromLuaValue,
};

// /null

// bool

void Bool_Serialize(Variant self, pugi::xml_node node) {
    node.text().set(self.get<bool>() ? "true" : "false");
}

Variant Bool_Deserialize(pugi::xml_node node) {
    return node.text().as_bool();
}

const std::string Bool_ToString(Variant self) {
    return self.get<bool>() ? "true" : "false";
}

const std::optional<Variant> Bool_FromString(std::string string) {
    return string[0] == 't' || string[0] == 'T' || string[0] == '1' || string[0] == 'y' || string[0] == 'Y';
}

void Bool_PushLuaValue(Variant self, lua_State* L) {
    lua_pushboolean(L, self.get<bool>());
}

result<Variant, LuaCastError> Bool_FromLuaValue(lua_State* L, int idx) {
    if (!lua_isboolean(L, idx))
        return LuaCastError(lua_typename(L, idx), "boolean");
    return Variant(lua_toboolean(L, idx));
}

const TypeDescriptor BOOL_TYPE {
    "bool",
    Bool_Serialize,
    Bool_Deserialize,
    Bool_ToString,
    Bool_FromString,
    Bool_PushLuaValue,
    Bool_FromLuaValue,
};

// /bool

// int

void Int_Serialize(Variant self, pugi::xml_node node) {
    node.text().set(self.get<int>());
}

Variant Int_Deserialize(pugi::xml_node node) {
    return node.text().as_int();
}

const std::string Int_ToString(Variant self) {
    return std::to_string(self.get<int>());
}

const std::optional<Variant> Int_FromString(std::string string) {
    char* endPos;
    int value = (int)std::strtol(string.c_str(), &endPos, 10);
    if (endPos == string.c_str()) return std::nullopt;
    return value;
}

void Int_PushLuaValue(Variant self, lua_State* L) {
    lua_pushinteger(L, self.get<int>());
}

result<Variant, LuaCastError> Int_FromLuaValue(lua_State* L, int idx) {
    if (!lua_isnumber(L, idx))
        return LuaCastError(lua_typename(L, idx), "integer");
    return Variant((int)lua_tonumber(L, idx));
}

const TypeDescriptor INT_TYPE {
    "int",
    Int_Serialize,
    Int_Deserialize,
    Int_ToString,
    Int_FromString,
    Int_PushLuaValue,
    Int_FromLuaValue,
};

// /int

// float

void Float_Serialize(Variant self, pugi::xml_node node) {
    node.text().set(self.get<float>());
}

Variant Float_Deserialize(pugi::xml_node node) {
    return node.text().as_float();
}

const std::string Float_ToString(Variant self) {
    std::stringstream stream;
    stream << std::noshowpoint << self.get<float>();
    return stream.str();
}

const std::optional<Variant> Float_FromString(std::string string) {
    char* endPos;
    float value = std::strtof(string.c_str(), &endPos);
    if (endPos == string.c_str()) return std::nullopt;
    return value;
}

void Float_PushLuaValue(Variant self, lua_State* L) {
    lua_pushnumber(L, self.get<float>());
}

result<Variant, LuaCastError> Float_FromLuaValue(lua_State* L, int idx) {
    if (!lua_isnumber(L, idx))
        return LuaCastError(lua_typename(L, idx), "float");
    return Variant((float)lua_tonumber(L, idx));
}

const TypeDescriptor FLOAT_TYPE {
    "float",
    Float_Serialize,
    Float_Deserialize,
    Float_ToString,
    Float_FromString,
    Float_PushLuaValue,
    Float_FromLuaValue,
};

// /float

// string

void String_Serialize(Variant self, pugi::xml_node node) {
    node.text().set(self.get<std::string>());
}

Variant String_Deserialize(pugi::xml_node node) {
    return node.text().as_string();
}

const std::string String_ToString(Variant self) {
    return self.get<std::string>();
}

const std::optional<Variant> String_FromString(std::string string) {
    return string;
}

void String_PushLuaValue(Variant self, lua_State* L) {
    lua_pushstring(L, self.get<std::string>().c_str());
}

result<Variant, LuaCastError> String_FromLuaValue(lua_State* L, int idx) {
    if (!lua_tostring(L, idx))
        return LuaCastError(lua_typename(L, idx), "string");
    return Variant(lua_tostring(L, idx));
}

const TypeDescriptor STRING_TYPE {
    "string",
    String_Serialize,
    String_Deserialize,
    String_ToString,
    String_FromString,
    String_PushLuaValue,
    String_FromLuaValue,
};

// /string