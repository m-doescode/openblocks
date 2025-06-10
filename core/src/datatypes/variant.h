#pragma once

#include <type_traits>
#include <variant>
#include <map>
#include "base.h"
#include "datatypes/color3.h"
#include "datatypes/enum.h"
#include "datatypes/ref.h"
#include "datatypes/signal.h"
#include "error/data.h"
#include "vector.h"
#include "cframe.h"

// #define __VARIANT_TYPE std::variant< \_
//         Null, \_
//         Bool, \_
//         Int, \_
//         Float, \_
//         String \_
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
    SignalConnectionRef,
    Enum,
    EnumItem
> __VARIANT_TYPE;

class Variant {
    __VARIANT_TYPE wrapped;
public:
    template <typename T, std::enable_if_t<std::is_constructible_v<__VARIANT_TYPE, T>, int> = 0> Variant(T obj) : wrapped(obj) {}
    template <typename T, std::enable_if_t<std::is_constructible_v<__VARIANT_TYPE, T>, int> = 0> T get() { return std::get<T>(wrapped); }
    std::string ToString() const;
    
    const TypeMeta GetTypeMeta() const;
    const TypeDesc* GetType() const;

    void Serialize(pugi::xml_node node) const;
    void PushLuaValue(lua_State* state) const;
    static result<Variant, DataParseError> Deserialize(pugi::xml_node node, const TypeMeta);
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

template <typename T, typename ...Args>
std::function<Variant(Args...)> toVariantGenerator(T(f)(Args...)) {
    return [f](Args... args) {
        return (Variant)f(args...);
    };
}

template <typename T, typename ...Args, typename ...E>
std::function<result<Variant, E...>(Args...)> toVariantGenerator(result<T, E...>(f)(Args...)) {
    return [f](Args... args) -> result<Variant, E...> {
        auto result = f(args...);
        if (result.isSuccess()) return (Variant)(result.success().value());
        return result.error().value();
    };
}

template <typename T, typename ...Args, typename ...E>
std::function<result<Variant, E...>(Args..., const TypeMeta)> toVariantGeneratorNoMeta(result<T, E...>(f)(Args...)) {
    return [f](Args... args, const TypeMeta) -> result<Variant, E...> {
        auto result = f(args...);
        if (result.isSuccess()) return (Variant)(result.success().value());
        return result.error().value();
    };
}

// Map of all data types to their type names
extern std::map<std::string, const TypeDesc*> TYPE_MAP;