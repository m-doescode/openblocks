#pragma once

#include <functional>
#include <memory>
#include <optional>
#include "datatypes/variant.h"
#include "datatypes.h"
#include "objects/base/member.h"

class Instance;

using PropertyGetter = std::function<Variant(std::shared_ptr<Instance>)>;
using PropertySetter = std::function<void(std::shared_ptr<Instance>, Variant)>;
using PropertyListener = std::function<void(std::shared_ptr<Instance>, std::string name, Variant oldValue, Variant newValue)>;

template <typename C>
using MemberPropertyListener = void (C::*)(std::string name, Variant oldValue, Variant newValue);

struct InstanceProperty {
    std::string name;

    TypeMeta type;
    PropertyFlags flags;
    std::string category;

    PropertyGetter getter;
    PropertySetter setter;
    std::optional<PropertyListener> listener;
};

template <typename T, typename C>
InstanceProperty def_property(std::string name, T C::* ref, PropertyFlags flags = 0, PropertyListener listener = {}) {
    return {
        name,
        type_meta_of<T>(),
        flags,
        "",

        [ref](std::shared_ptr<Instance> instance) {
            auto obj = std::dynamic_pointer_cast<C>(instance);
            return obj.get()->*ref;
        },
        [ref](std::shared_ptr<Instance> instance, Variant value) {
            auto obj = std::dynamic_pointer_cast<C>(instance);
            obj.get()->*ref = value.get<T>();
        },
        listener
    };
}

template <typename T, typename C>
InstanceProperty def_property(std::string name, T C::* ref, PropertyFlags flags, MemberPropertyListener<C> listener) {
    return def_property(name, ref, flags, [listener](std::shared_ptr<Instance> instance, std::string name, Variant oldValue, Variant newValue) {
        auto obj = std::dynamic_pointer_cast<C>(instance);
        obj->*listener(name, oldValue, newValue);
    });
}

struct set_property_category {
    std::string name;
};