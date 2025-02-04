#pragma once

#include "base.h"
#include "vector.h"
#include <variant>

// #define __VARIANT_TYPE std::variant< \
//         Null, \
//         Bool, \
//         Int, \
//         Float, \
//         String \
//     >

namespace Data {
    typedef std::variant<
        Null,
        Bool,
        Int,
        Float,
        String,
        Vector3
    > __VARIANT_TYPE;

    class Variant {
        __VARIANT_TYPE wrapped;
    public:
        template <typename T> Variant(T obj) : wrapped(obj) {}
        template <typename T> T get() { return std::get<T>(wrapped); }
        Data::String ToString() const;
    };
}