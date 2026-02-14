#pragma once

#include "objectmodel/property.h"

// TODO: Maybe find a better solution?
// Because def_property refers to Instance::Type(), we have to feed it this way instead
template <typename T, typename C>
InstanceProperty def_property_apex(const InstanceType& typeMeta, std::string name, T C::* ref, PropertyFlags flags = 0, PropertyListener listener = {}) {
    return {
        name,
        &typeMeta,
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