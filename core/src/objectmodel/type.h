#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include "property.h"
#include "signal.h"
#include "method.h"

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
    std::string explorerIcon;
    std::optional<InstanceConstructor> constructor;

    // Members
    std::map<std::string, InstanceProperty> properties;
    std::map<std::string, InstanceSignal> signalSources; // Can't use signals because it's #defined by Qt
    std::map<std::string, InstanceMethod> methods;

    inline bool operator==(const InstanceType& other) const {
        return this == &other;
    }
};

struct __make_instance_type_temps {
    std::string lastCategory;
};

struct set_explorer_icon {
    std::string explorerIcon;
};

// Members

inline void __instance_type_add_member(InstanceType& type, __make_instance_type_temps& temps, InstanceProperty property) {
    property.category = temps.lastCategory;
    type.properties[property.name] = property;
}

inline void __instance_type_add_member(InstanceType& type, __make_instance_type_temps& temps, InstanceSignal signal) {
    type.signalSources[signal.name] = signal;
}

inline void __instance_type_add_member(InstanceType& type, __make_instance_type_temps& temps, InstanceMethod method) {
    type.methods[method.name] = method;
}

// Flags

inline void __instance_type_add_member(InstanceType& type, __make_instance_type_temps& temps, set_property_category category) {
    temps.lastCategory = category.name;
}

inline void __instance_type_add_member(InstanceType& type, __make_instance_type_temps& temps, set_explorer_icon explorerIcon) {
    type.explorerIcon = explorerIcon.explorerIcon;
}

template <typename T>
std::shared_ptr<T> __new_instance() {
    std::shared_ptr<T> obj = std::make_shared<T>();
    if (obj->name == "") obj->name = T::Type().className;
    return obj;
}

template <typename T>
std::optional<InstanceConstructor> __get_instance_constructor() {
    if constexpr (std::is_constructible_v<T>) {
        return []() { return __new_instance<T>(); };
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
    auto& super = B::Type();
    type.super = &super;
    type.explorerIcon = super.explorerIcon;
    type.properties = super.properties;
    type.signalSources = super.signalSources;
    type.methods = super.methods;

    __make_instance_type_temps temps;
    (__instance_type_add_member(type, temps, args), ...);

    return type;
}

template <typename T, typename B = Instance, typename ...Args /* TODO: Add SFINAE */ >
const InstanceType make_instance_type(std::string name, Args... args) {
    return make_instance_type<T, B, Args...>(name, 0, args...);
}