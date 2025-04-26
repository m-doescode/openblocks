#pragma once

#include "../../datatypes/base.h"
#include "datatypes/meta.h"
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <variant>

class Instance;

struct FieldCodec {
    void (*write)(Data::Variant source, void* destination);
    Data::Variant (*read)(void* source);
};

template <typename T, typename U>
constexpr FieldCodec fieldCodecOf() {
    return FieldCodec {
        .write = [](Data::Variant source, void* destination) {
            *(U*)destination = (U)source.get<T>();
        },
        .read = [](void* source) -> Data::Variant {
            return T(*(U*)source);
        },
    };
}

template <typename T>
constexpr FieldCodec fieldCodecOf() {
    return FieldCodec {
        .write = [](Data::Variant source, void* destination) {
            *(T*)destination = source.get<T>();
        },
        .read = [](void* source) -> Data::Variant {
            return *(T*)source;
        },
    };
}

template <typename T>
std::function<void(std::string name)> memberFunctionOf(void(T::*func)(std::string), T* obj) {
    return std::bind(func, obj, std::placeholders::_1);
}

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
};

const int PROPERTY_CATEGORY_MAX = PROP_CATEGORY_SURFACE;

struct PropertyMeta {
    const Data::TypeInfo* type;
    PropertyFlags flags;
    PropertyCategory category = PROP_CATEGORY_DATA;
};

typedef std::variant<PropertyMeta> MemberMeta;