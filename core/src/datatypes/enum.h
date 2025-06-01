#pragma once

#include <optional>
#include <string>
#include <vector>

struct _EnumData {
    std::pair<int, std::string>* values;
    int count;
};

class EnumItem;

class Enum {
    _EnumData* data;
public:
    Enum(_EnumData*);

    std::vector<EnumItem> GetEnumItems();
    std::optional<EnumItem> FromName(std::string);
    std::optional<EnumItem> FromValue(int);
};

class EnumItem {
    _EnumData* parentData;
    std::string name;
    int value;
public:
    EnumItem(_EnumData*, std::string, int);
    inline std::string Name() { return this->name; }
    inline int Value() { return this->value; }
    inline Enum EnumType() { return Enum(this->parentData); }
};