#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include "property.h"

using InstanceFlags = int;
const InstanceFlags INSTANCE_NOTCREATABLE = 1 << 0; // This instance should only be instantiated in special circumstances
                                                    // (i.e. by DataModel) and should be creatable directly via any API 
const InstanceFlags INSTANCE_SERVICE = 1 << 1; // This instance is a service
const InstanceFlags INSTANCE_HIDDEN = 1 << 2; // This instance should be hidden from the explorer

using InstanceConstructor = std::function<std::shared_ptr<Instance>()>;

struct InstanceType {
    std::string className;
    InstanceFlags flags;
    const InstanceType* super;
    std::optional<InstanceConstructor> constructor;

    // Members
    std::map<std::string, InstanceProperty> properties;
};

struct __make_instance_type_temps {
    std::string lastCategory;
};

inline void __instance_type_add_member(InstanceType& type, __make_instance_type_temps& temps, InstanceProperty property) {
    // TODO: Add error checks here

    property.category = temps.lastCategory;
    type.properties[property.name] = property;
}

inline void __instance_type_add_member(InstanceType& type, __make_instance_type_temps& temps, set_property_category category) {
    temps.lastCategory = category.name;
}

template <typename T>
std::optional<InstanceConstructor> __get_instance_constructor() {
    if constexpr (!std::is_abstract_v<T>) {
        return []() { return std::make_shared<T>(); };
    } else {
        return {};
    }
}

template <typename T, typename B = Instance, typename ...Args /* TODO: Add SFINAE */ >
const InstanceType make_instance_type(std::string name, InstanceFlags flags, Args... args) {
    InstanceType type;
    type.className = name;
    type.flags = flags;
    type.constructor = __get_instance_constructor<T>();

    // Add members from parent type
    auto super = B::Type();
    type.super = &super;
    type.properties = super.properties;

    __make_instance_type_temps temps;
    (__instance_type_add_member(type, temps, args), ...);

    return type;
}

template <typename T, typename B = Instance, typename ...Args /* TODO: Add SFINAE */ >
const InstanceType make_instance_type(std::string name, Args... args) {
    return make_instance_type<T, B>(name, 0, args...);
}