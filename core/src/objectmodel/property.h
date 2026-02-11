#pragma once

#include <functional>
#include <memory>
#include <optional>
#include "datatypes/variant.h"
#include "datatypes.h"
#include "logger.h"
#include "objects/base/member.h"

class Instance;

using PropertyGetter = std::function<Variant(std::shared_ptr<Instance>)>;
using PropertySetter = std::function<void(std::shared_ptr<Instance>, Variant)>;
using PropertyListener = std::function<void(std::shared_ptr<Instance>, std::string name, Variant oldValue, Variant newValue)>;

template <typename C>
using MemberPropertyListener = void (C::*)(std::string name, Variant oldValue, Variant newValue);

template <typename T, typename C>
using PropertySupplier = std::function<T(C*)>;

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

// Separate C and C2 as the member function may be inherited
template <typename T, typename C, typename C2>
InstanceProperty def_property(std::string name, T C::* ref, PropertyFlags flags, MemberPropertyListener<C2> listener) {
    return def_property(name, ref, flags, [listener](std::shared_ptr<Instance> instance, std::string name, Variant oldValue, Variant newValue) {
        auto obj = std::dynamic_pointer_cast<C>(instance);
        (obj.get()->*listener)(name, oldValue, newValue);
    });
}

// Define through getter/setter
template <typename T, typename C>
InstanceProperty def_property(std::string name, PropertySupplier<T, C> supplier, PropertyFlags flags = 0, PropertyListener listener = {}) {
    return {
        name,
        type_meta_of<T>(),
        flags | PROP_READONLY,
        "",

        [supplier](std::shared_ptr<Instance> instance) {
            auto obj = std::dynamic_pointer_cast<C>(instance);
            return supplier(obj.get());
        },
        [](std::shared_ptr<Instance> instance, Variant value) {
            Logger::fatalError("Property cannot be assigned");
            panic();
        },
        listener
    };
}

struct set_property_category {
    std::string name;
};