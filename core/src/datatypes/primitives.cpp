#include "primitives.h"
#include "datatypes/base.h"
#include "datatypes/enum.h"
#include "error/data.h"
#include "logger.h"
#include "variant.h"
#include <pugixml.hpp>
#include "luaapis.h" // IWYU pragma: keep
#include <sstream>

TypeMeta::TypeMeta(const TypeMeta& src) : descriptor(src.descriptor) {
    if (src.descriptor == &EnumItem::TYPE) {
        enum_ = src.enum_;
    } else if (src.descriptor == &InstanceRef::TYPE) {
        instType = src.instType;
    }
}

TypeMeta& TypeMeta::operator=(const TypeMeta& src) {
    this->descriptor = src.descriptor;
    if (src.descriptor == &EnumItem::TYPE) {
        enum_ = src.enum_;
    } else if (src.descriptor == &InstanceRef::TYPE) {
        instType = src.instType;
    }
    return *this;
}

TypeMeta::TypeMeta() : descriptor(&NULL_TYPE) {}

// null

void Null_Serialize(Variant self, pugi::xml_node node) {
    node.text().set("null");
}

result<Variant, DataParseError> Null_Deserialize(pugi::xml_node node, const TypeMeta) {
    return std::monostate();
}

const std::string Null_ToString(Variant self) {
    return "null";
}

result<Variant, DataParseError> Null_FromString(std::string string, const TypeMeta) {
    return std::monostate();
}

void Null_PushLuaValue(Variant self, lua_State* L) {
    lua_pushnil(L);
}

result<Variant, LuaCastError> Null_FromLuaValue(lua_State* L, int idx) {
    return Variant(std::monostate());
}

const TypeDesc NULL_TYPE {
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

result<Variant, DataParseError> Bool_Deserialize(pugi::xml_node node, const TypeMeta) {
    return node.text().as_bool();
}

const std::string Bool_ToString(Variant self) {
    return self.get<bool>() ? "true" : "false";
}

result<Variant, DataParseError> Bool_FromString(std::string string, const TypeMeta) {
    return string[0] == 't' || string[0] == 'T' || string[0] == '1' || string[0] == 'y' || string[0] == 'Y';
}

void Bool_PushLuaValue(Variant self, lua_State* L) {
    lua_pushboolean(L, self.get<bool>());
}

result<Variant, LuaCastError> Bool_FromLuaValue(lua_State* L, int idx) {
    if (!lua_isboolean(L, idx))
        return LuaCastError(lua_typename(L, lua_type(L, idx)), "boolean");
    return Variant((bool)lua_toboolean(L, idx));
}

const TypeDesc BOOL_TYPE {
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

result<Variant, DataParseError> Int_Deserialize(pugi::xml_node node, const TypeMeta) {
    return node.text().as_int();
}

const std::string Int_ToString(Variant self) {
    return std::to_string(self.get<int>());
}

result<Variant, DataParseError> Int_FromString(std::string string, const TypeMeta) {
    char* endPos;
    int value = (int)std::strtol(string.c_str(), &endPos, 10);
    if (endPos == string.c_str()) return DataParseError(string, "int");
    return value;
}

void Int_PushLuaValue(Variant self, lua_State* L) {
    lua_pushinteger(L, self.get<int>());
}

result<Variant, LuaCastError> Int_FromLuaValue(lua_State* L, int idx) {
    if (!lua_isnumber(L, idx))
        return LuaCastError(lua_typename(L, lua_type(L, idx)), "integer");
    return Variant((int)lua_tonumber(L, idx));
}

const TypeDesc INT_TYPE {
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

result<Variant, DataParseError> Float_Deserialize(pugi::xml_node node, const TypeMeta) {
    return node.text().as_float();
}

const std::string Float_ToString(Variant self) {
    std::stringstream stream;
    stream << std::noshowpoint << self.get<float>();
    return stream.str();
}

result<Variant, DataParseError> Float_FromString(std::string string, const TypeMeta) {
    char* endPos;
    float value = std::strtof(string.c_str(), &endPos);
    if (endPos == string.c_str()) return DataParseError(string, "float");
    return value;
}

void Float_PushLuaValue(Variant self, lua_State* L) {
    lua_pushnumber(L, self.get<float>());
}

result<Variant, LuaCastError> Float_FromLuaValue(lua_State* L, int idx) {
    if (!lua_isnumber(L, idx))
        return LuaCastError(lua_typename(L, lua_type(L, idx)), "float");
    return Variant((float)lua_tonumber(L, idx));
}

const TypeDesc FLOAT_TYPE {
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

result<Variant, DataParseError> String_Deserialize(pugi::xml_node node, const TypeMeta) {
    return node.text().as_string();
}

const std::string String_ToString(Variant self) {
    return self.get<std::string>();
}

result<Variant, DataParseError> String_FromString(std::string string, const TypeMeta) {
    return string;
}

void String_PushLuaValue(Variant self, lua_State* L) {
    lua_pushstring(L, self.get<std::string>().c_str());
}

result<Variant, LuaCastError> String_FromLuaValue(lua_State* L, int idx) {
    if (!lua_tostring(L, idx))
        return LuaCastError(lua_typename(L, lua_type(L, idx)), "string");
    return Variant(lua_tostring(L, idx));
}

const TypeDesc STRING_TYPE {
    "string",
    String_Serialize,
    String_Deserialize,
    String_ToString,
    String_FromString,
    String_PushLuaValue,
    String_FromLuaValue,
};

// /string

// array

TypeMeta::TypeMeta(const std::shared_ptr<const TypeMeta> nestedType) : nestedType(nestedType) {}

void Array_Serialize(Variant self, pugi::xml_node node) {
    Logger::fatalError("Array serialization is not yet implemented");
    // node.text().set(self.get<std::vector<Variant>>());
}

result<Variant, DataParseError> Array_Deserialize(pugi::xml_node node, const TypeMeta) {
    Logger::fatalError("Array serialization is not yet implemented");
    return DataParseError(node.name(), "array<NIY>");
}

const std::string Array_ToString(Variant self) {
    auto vec = self.get<std::vector<Variant>>();
    std::stringstream buf;
    buf << "[";
    for (auto&& item : vec) {
        buf << item.ToString() << ",";
    }
    buf << "]";
    return buf.str();
}

result<Variant, DataParseError> Array_FromString(std::string string, const TypeMeta) {
    Logger::fatalError("Array from string is not yet implemented");
    return DataParseError(string, "array<NIY>");
}

void Array_PushLuaValue(Variant self, lua_State* L) {
    auto vec = self.get<std::vector<Variant>>();
    lua_createtable(L, vec.size(), 0);
    for (size_t i = 0; i < vec.size(); i++) {
        auto&& item = vec[i];
        item.PushLuaValue(L);
        lua_rawseti(L, -2, i+1);
    }
}

result<Variant, LuaCastError> Array_FromLuaValue(lua_State* L, int idx) {
    Logger::fatalError("Array from lua is not yet implemented");
    return LuaCastError(lua_typename(L, lua_type(L, idx)), "array<NIY>");
    // std::vector<Variant> vec;
    // luaL_checktype(L, idx, LUA_TTABLE);
    // for (size_t i = 0; i < lua_objlen(L, idx); i++) {
    //     Variant value = 
    // }
    // return vec;
}

const TypeDesc ARRAY_TYPE {
    "array",
    Array_Serialize,
    Array_Deserialize,
    Array_ToString,
    Array_FromString,
    Array_PushLuaValue,
    Array_FromLuaValue,
};

// /array