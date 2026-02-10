#pragma once

#include <map>
#include <string>
#include "objects/base/instance.h"
#include "property.h"

class Instance2; // TEMPORARY

struct InstanceType2 {
    std::string className;
    InstanceFlags flags;
    std::map<std::string, InstanceProperty> properties;
};

inline void __instance_type_add_member(InstanceType2& type, InstanceProperty property) {
    // TODO: Add error checks here

    type.properties[property.name] = property;
}

template <typename T, typename ...Args /* TODO: Add SFINAE */ >
const InstanceType2 make_instance_type(std::string name, InstanceFlags flags, Args... args) {
    InstanceType2 type;
    type.className = name;

    (__instance_type_add_member(type, args), ...);

    return type;
}

template <typename T, typename ...Args /* TODO: Add SFINAE */ >
const InstanceType2 make_instance_type(std::string name, Args... args) {
    return make_instance_type<T>(name, 0, args...);
}

// TEMPORARY

class Instance2 {
    virtual const InstanceType2& GetType() = 0;
};