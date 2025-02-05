#pragma once

#include <variant>
#include "base.h"
#include "vector.h"
#include "cframe.h"

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
        Vector3,
        CFrame
    > __VARIANT_TYPE;

    class Variant {
        __VARIANT_TYPE wrapped;
    public:
        template <typename T> Variant(T obj) : wrapped(obj) {}
        template <typename T> T get() { return std::get<T>(wrapped); }
        Data::String ToString() const;
    };
}