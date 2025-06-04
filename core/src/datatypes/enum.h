#pragma once

#include "base.h"
#include <optional>
#include <string>
#include <vector>
#include "datatypes/annotation.h"
#include "error/data.h"
#include "lua.h"

struct _EnumData {
    std::string name;
    std::pair<int, std::string>* values;
    int count;
};

class EnumItem;

class DEF_DATA Enum {
    _EnumData* data;
public:
    Enum(_EnumData*);

    static const TypeDesc TYPE;

    inline _EnumData* InternalType() const { return this->data; };
    std::vector<EnumItem> GetEnumItems() const;
    std::optional<EnumItem> FromName(std::string) const;
    std::optional<EnumItem> FromValue(int) const;

    std::string ToString() const;
    void PushLuaValue(lua_State*) const;
    static result<Variant, LuaCastError> FromLuaValue(lua_State*, int);
};

class DEF_DATA EnumItem {
    _EnumData* parentData;
    std::string name;
    int value;
public:
    EnumItem(_EnumData*, std::string, int);
    
    static const TypeDesc TYPE;

    inline std::string Name() const { return this->name; }
    inline int Value() const { return this->value; }
    inline Enum EnumType() const { return Enum(this->parentData); }

    static result<EnumItem, DataParseError> FromString(std::string, const TypeMeta);
    std::string ToString() const;
    void Serialize(pugi::xml_node) const;
    static result<EnumItem, DataParseError> Deserialize(pugi::xml_node, const TypeMeta);
    void PushLuaValue(lua_State*) const;
    static result<Variant, LuaCastError> FromLuaValue(lua_State*, int);
};