#pragma once

#include "datatypes/base.h"
#include <variant>

class Instance;


typedef int PropertyFlags;
const PropertyFlags PROP_HIDDEN = 1 << 0; // Hidden from the editor
const PropertyFlags PROP_NOSAVE = 1 << 1; // Do not serialize
const PropertyFlags PROP_UNIT_FLOAT = 1 << 2; // Float between 0 and 1
const PropertyFlags PROP_READONLY = 1 << 3; // Read only property, do not write

enum PropertyCategory {
    PROP_CATEGORY_APPEARENCE,
    PROP_CATEGORY_DATA,
    PROP_CATEGORY_BEHAVIOR,
    PROP_CATEGORY_PART,
    PROP_CATEGORY_SURFACE,
    PROP_CATEGORY_SURFACE_INPUT,
};

const int PROPERTY_CATEGORY_MAX = PROP_CATEGORY_SURFACE_INPUT;

struct PropertyMeta {
    const TypeMeta type;
    PropertyFlags flags;
    PropertyCategory category = PROP_CATEGORY_DATA;
};

typedef std::variant<PropertyMeta> MemberMeta;