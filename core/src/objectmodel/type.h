#pragma once

#include <map>
#include <memory>
#include <string>
#include <functional>
#include "datatypes/variant.h"

class Instance2; // TEMPORARY

#define INSTANCE_HEADER_SOURCE  \
public: \
    static inline const InstanceType2& Type() { \
        static const InstanceType2 type = __buildType(); \
        return type; \
    } \
\
    inline const InstanceType2& GetType() override { \
        return Type();\
    } \
private:

using PropertyGetter = std::function<Variant(std::shared_ptr<Instance2>)>;
using PropertySetter = std::function<void(std::shared_ptr<Instance2>, Variant)>;

struct InstanceProperty {
    std::string name;

    PropertyGetter getter;
    PropertySetter setter;
};

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

template <typename T, typename C>
InstanceProperty def_property(std::string name, T C::* ref) {
    return {
        name,
        [ref](std::shared_ptr<Instance2> instance) {
            auto obj = std::dynamic_pointer_cast<C>(instance);
            return obj.get()->*ref;
        },
        [ref](std::shared_ptr<Instance2> instance, Variant value) {
            auto obj = std::dynamic_pointer_cast<C>(instance);
            obj.get()->*ref = value.get<T>();
        }
    };
}

// TEMPORARY

class Instance2 {
    virtual const InstanceType2& GetType() = 0;
};