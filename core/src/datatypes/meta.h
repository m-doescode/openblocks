#pragma once

#include "base.h"
#include <map>
#include <type_traits>
#include <typeindex>

extern std::map<std::type_index, const Enum*> ENUM_TYPE;
extern std::map<std::type_index, TypeMeta> INTEGRAL_TYPE_META;

class Instance;

template <typename T> TypeMeta type_meta_of() {
    if constexpr (std::is_base_of_v<Instance, T>) {
        // Is instance
        return T::Type();
    } else if constexpr (std::is_enum_v<T>) {
        // Is enum
        return ENUM_TYPE[std::type_index(typeid(T))];
    } else {
        // Is integral type
        return INTEGRAL_TYPE_META[std::type_index(typeid(T))];
    }
}