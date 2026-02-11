#pragma once

#include "datatypes/base.h"
#include <variant>

class Instance;


typedef int PropertyFlags;
const PropertyFlags PROP_HIDDEN = 1 << 0; // Hidden from the editor
const PropertyFlags PROP_NOSAVE = 1 << 1; // Do not serialize
const PropertyFlags PROP_UNIT_FLOAT = 1 << 2; // Float between 0 and 1
const PropertyFlags PROP_READONLY = 1 << 3; // Read only property, do not write

struct PropertyMeta {
    const TypeMeta type;
    PropertyFlags flags;
    std::string category;
};

typedef std::variant<PropertyMeta> MemberMeta;