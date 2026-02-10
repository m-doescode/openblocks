#pragma once

#include <map>
#include <string>
#include "property.h"

class Instance2; // TEMPORARY

struct InstanceType2 {
    std::string className;
    std::map<std::string, InstanceProperty> properties;
};

inline void __instance_type_add_member(InstanceType2& type, InstanceProperty property) {
    // TODO: Add error checks here

    type.properties[property.name] = property;
}

template <typename T, typename ...Args /* TODO: Add SFINAE */ >
const InstanceType2 make_instance_type(std::string name, Args... args) {
    InstanceType2 type;
    type.className = name;

    (__instance_type_add_member(type, args), ...);

    return type;
}

// TEMPORARY

class Instance2 {
    virtual const InstanceType2& GetType() = 0;
};