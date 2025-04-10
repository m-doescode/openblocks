#pragma once

#include <variant>
#include <map>
#include "base.h"
#include "datatypes/color3.h"
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
        CFrame,
        Color3
    > __VARIANT_TYPE;

    class Variant {
        __VARIANT_TYPE wrapped;
    public:
        template <typename T> Variant(T obj) : wrapped(obj) {}
        template <typename T> T get() { return std::get<T>(wrapped); }
        Data::String ToString() const;
        
        void Serialize(pugi::xml_node node) const;
        static Data::Variant Deserialize(pugi::xml_node node);
    };

    // Map of all data types to their type names
    extern std::map<std::string, const TypeInfo*> TYPE_MAP;
}