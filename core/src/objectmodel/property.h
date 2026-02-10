#pragma once

#include <functional>
#include "datatypes/variant.h"
#include "datatypes.h"

class Instance2;

using PropertyGetter = std::function<Variant(std::shared_ptr<Instance2>)>;
using PropertySetter = std::function<void(std::shared_ptr<Instance2>, Variant)>;

struct InstanceProperty {
    std::string name;

    TypeMeta type;

    PropertyGetter getter;
    PropertySetter setter;
};

template <typename T, typename C>
InstanceProperty def_property(std::string name, T C::* ref) {
    return {
        name,
        type_meta_of<T>(),
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
