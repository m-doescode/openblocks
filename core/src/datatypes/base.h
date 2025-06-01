#pragma once

#include <string>
#include <functional>
#include <optional>
#include "datatypes/enum.h"
#include "error/result.h"
#include "error/data.h"

extern "C" { typedef struct lua_State lua_State; }

namespace pugi { class xml_node; };

class Variant;
typedef std::function<void(Variant, pugi::xml_node)> Serializer;
typedef std::function<Variant(pugi::xml_node)> Deserializer;
typedef std::function<std::string(Variant)> ToString;
typedef std::function<std::optional<Variant>(std::string)> FromString;
typedef std::function<result<Variant, LuaCastError>(lua_State*, int idx)> FromLuaValue;
typedef std::function<void(Variant self, lua_State*)> PushLuaValue;

struct TypeDescriptor {
    std::string name;
    Serializer serializer;
    Deserializer deserializer;
    ToString toString;
    FromString fromString;
    PushLuaValue pushLuaValue;
    FromLuaValue fromLuaValue;
};

enum DataType {
    // This distinction is not currently useful, so to
    // minimize complexity, there will only be two categories
    // (for now)
    //DATA_PRIMITIVE,
    //DATA_COMPOUND,
    DATA_VALUE,
    DATA_ENUM,
};

struct TypeInfo {
    DataType type;
    union {
        const TypeDescriptor* descriptor;
        _EnumData* enumData;
    };
    
    inline TypeInfo(const TypeDescriptor* descriptor) : type(DATA_VALUE), descriptor(descriptor) {}
};