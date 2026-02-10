#pragma once

#include <map>
#include <string>
#include "objects/base/instance.h"
#include "property.h"

class Instance2; // TEMPORARY

struct InstanceType2 {
    std::string className;
    const InstanceType2* super;
    InstanceFlags flags;
    std::map<std::string, InstanceProperty> properties;
};

struct __make_instance_type_temps {
    std::string lastCategory;
};

inline void __instance_type_add_member(InstanceType2& type, __make_instance_type_temps& temps, InstanceProperty property) {
    // TODO: Add error checks here

    property.category = temps.lastCategory;
    type.properties[property.name] = property;
}

inline void __instance_type_add_member(InstanceType2& type, __make_instance_type_temps& temps, set_property_category category) {
    temps.lastCategory = category.name;
}

template <typename T, typename B = Instance2, typename ...Args /* TODO: Add SFINAE */ >
const InstanceType2 make_instance_type(std::string name, InstanceFlags flags, Args... args) {
    InstanceType2 type;
    type.className = name;

    // Add members from parent type
    auto super = B::Type();
    type.super = &super;
    type.properties = super.properties;

    __make_instance_type_temps temps;
    (__instance_type_add_member(type, temps, args), ...);

    return type;
}

template <typename T, typename B = Instance2, typename ...Args /* TODO: Add SFINAE */ >
const InstanceType2 make_instance_type(std::string name, Args... args) {
    return make_instance_type<T, B>(name, 0, args...);
}

// TEMPORARY

class Instance2 {
    static InstanceType2 __buildType() {
        InstanceType2 type;
        type.className = "Instance";
        type.super = nullptr;
        type.flags = INSTANCE_NOTCREATABLE;
        return type;
    }
public:
    static const InstanceType2& Type() {
        static InstanceType2 type = __buildType();
        return type;
    }; 
    virtual const InstanceType2& GetType() = 0;
};