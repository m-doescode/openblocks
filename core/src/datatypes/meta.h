#pragma once

#include "base.h"
#include <map>
#include <memory>
#include <type_traits>
#include <typeindex>

class Instance;

namespace detail {
    template <typename T> struct is_instance_ptr : std::integral_constant<bool, false> { };
    template <typename T> struct is_instance_ptr<std::shared_ptr<T>> : std::enable_if_t<std::is_base_of_v<Instance, T>, std::integral_constant<bool, true>> { };
    template <typename T> struct is_instance_ptr<std::weak_ptr<T>> : std::enable_if_t<std::is_base_of_v<Instance, T>, std::integral_constant<bool, true>> { };

    template <typename T>
    inline constexpr bool is_instance_ptr_v = is_instance_ptr<T>::value;
};

extern std::map<std::type_index, const Enum*> ENUM_TYPE;
extern std::map<std::type_index, TypeMeta> INTEGRAL_TYPE_META;
// Map of all data types to their type names
extern std::map<std::string, const TypeDesc*> TYPE_BY_NAME;

template <typename T> TypeMeta type_meta_of() {
    if constexpr (detail::is_instance_ptr_v<T>) {
        // Is instance
        return &T::element_type::Type();
    } else if constexpr (std::is_enum_v<T>) {
        // Is enum
        return ENUM_TYPE[std::type_index(typeid(T))];
    } else {
        // Is integral type
        return INTEGRAL_TYPE_META[std::type_index(typeid(T))];
    }
}