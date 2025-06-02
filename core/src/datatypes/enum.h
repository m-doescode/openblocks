#pragma once

#include "base.h"
#include <optional>
#include <string>
#include <vector>
#include "lua.h"

struct _EnumData {
    std::pair<int, std::string>* values;
    std::string name;
    int count;
};

class EnumItem;

class Enum {
    _EnumData* data;
public:
    Enum(_EnumData*);

    static const TypeDescriptor TYPE;

    inline _EnumData* InternalType() const { return this->data; };
    std::vector<EnumItem> GetEnumItems() const;
    std::optional<EnumItem> FromName(std::string) const;
    std::optional<EnumItem> FromValue(int) const;

    std::string ToString();
    void PushLuaValue(lua_State*);
    static Variant FromLuaValue(lua_State*, int);
};

class EnumItem {
    _EnumData* parentData;
    std::string name;
    int value;
public:
    EnumItem(_EnumData*, std::string, int);
    
    static const TypeDescriptor TYPE;

    inline std::string Name() const { return this->name; }
    inline int Value() const { return this->value; }
    inline Enum EnumType() const { return Enum(this->parentData); }
};