#pragma once

#include <string>
#include <functional>
#include <optional>
#include "error/result.h"
#include "error/data.h"

extern "C" { typedef struct lua_State lua_State; }

namespace pugi { class xml_node; };

class Variant;
struct TypeMeta;

typedef std::function<void(Variant, pugi::xml_node)> Serialize;
typedef std::function<result<Variant, DataParseError>(pugi::xml_node, const TypeMeta)> Deserialize;
typedef std::function<std::string(Variant)> ToString;
typedef std::function<result<Variant, DataParseError>(std::string, const TypeMeta)> FromString;
typedef std::function<result<Variant, LuaCastError>(lua_State*, int idx)> FromLuaValue;
typedef std::function<void(Variant self, lua_State*)> PushLuaValue;

// Describes a concrete type
struct TypeDesc {
    std::string name;
    Serialize serialize;
    Deserialize deserialize;
    ToString toString;
    FromString fromString;
    PushLuaValue pushLuaValue;
    FromLuaValue fromLuaValue;
};

class Enum;
struct InstanceType;

// Describes a meta-type, which consists of a concrete type, and some generic argument.
struct TypeMeta {
    const TypeDesc* descriptor;
    union {
        const Enum* enum_; // Applicable for EnumItem
        const InstanceType* instType; // Applicable for InstanceRef
    };

    inline TypeMeta(const TypeDesc* descriptor) : descriptor(descriptor) {}
    TypeMeta(const Enum*);
    TypeMeta(const InstanceType*);
};