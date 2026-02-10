#pragma once

#include <functional>
#include "datatypes/variant.h"
#include "datatypes.h"
#include "objects/base/member.h"

class Instance;

using PropertyGetter = std::function<Variant(std::shared_ptr<Instance>)>;
using PropertySetter = std::function<void(std::shared_ptr<Instance>, Variant)>;

struct InstanceProperty {
    std::string name;

    TypeMeta type;
    PropertyFlags flags;
    std::string category;

    PropertyGetter getter;
    PropertySetter setter;
};

template <typename T, typename C>
InstanceProperty def_property(std::string name, T C::* ref, PropertyFlags flags = 0) {
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
        }
    };
}

struct set_property_category {
    std::string name;
};