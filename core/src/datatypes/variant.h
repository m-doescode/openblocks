#pragma once

#include <variant>
#include <map>
#include "base.h"
#include "datatypes/color3.h"
#include "datatypes/ref.h"
#include "datatypes/signal.h"
#include "vector.h"
#include "cframe.h"

// #define __VARIANT_TYPE std::variant< \
//         Null, \
//         Bool, \
//         Int, \
//         Float, \
//         String \
//     >

typedef std::variant<
    std::monostate,
    bool,
    int,
    float,
    std::string,
    Vector3,
    CFrame,
    Color3,
    InstanceRef,
    SignalRef,
    SignalConnectionRef
> __VARIANT_TYPE;

class Variant {
    __VARIANT_TYPE wrapped;
public:
    template <typename T> Variant(T obj) : wrapped(obj) {}
    template <typename T> T get() { return std::get<T>(wrapped); }
    std::string ToString() const;
    
    void Serialize(pugi::xml_node node) const;
    void PushLuaValue(lua_State* state) const;
    static Variant Deserialize(pugi::xml_node node);
};

template <typename T, typename R, typename ...Args>
std::function<R(Variant, Args...)> toVariantFunction(R(T::*f)(Args...)) {
    return [f](Variant var, Args... args) {
        return (var.get<T>().*f)(args...);
    };
}

template <typename T, typename R, typename ...Args>
std::function<R(Variant, Args...)> toVariantFunction(R(T::*f)(Args...) const) {
    return [f](Variant var, Args... args) {
        return (var.get<T>().*f)(args...);
    };
}

// Map of all data types to their type names
extern std::map<std::string, const TypeInfo*> TYPE_MAP;