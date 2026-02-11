#pragma once

#include <memory>
#include <type_traits>
#include <variant>
#include <map>
#include "base.h"
#include "datatypes/color3.h"
#include "datatypes/enum.h"
#include "datatypes/ref.h"
#include "datatypes/signal.h"
#include "datatypes/enummeta.h"
#include "error/data.h"
#include "vector.h"
#include "cframe.h"

using __VARIANT_TYPE = std::variant<
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
>;

// Is this a valid generic type
template <typename T> using __is_simple_variant_type = std::enable_if<std::is_constructible_v<__VARIANT_TYPE, T>>;
template <typename T, typename R> using __is_simple_variant_type_t = std::enable_if_t<std::is_constructible_v<__VARIANT_TYPE, T>, R>;

// Convert any type to a generic type (T -> T, EnumItem, InstanceRef)
template <typename T> __is_simple_variant_type_t<T, T> __to_variant(T& t) { return t; }
template <typename T> InstanceRef __to_variant(std::shared_ptr<T>& t) { return InstanceRef(t); }
template <typename T> InstanceRef __to_variant(std::weak_ptr<T>& t) { return InstanceRef(t); }
template <typename T> std::enable_if_t<std::is_enum_v<T>, EnumItem> __to_variant(T& t) { return enum_type_of_t<T>().value.FromValueInternal((int)t); }

// Convert generic type into specialized type (* -> _enum_, std::shared_ptr<_instance_>)
template <typename T> __is_simple_variant_type_t<T, T> __from_variant(__VARIANT_TYPE& wrapped) { return std::get<T>(wrapped); }
template <typename T> std::enable_if_t<std::is_enum_v<T>, T> __from_variant(__VARIANT_TYPE& wrapped) { return (T)std::get<EnumItem>(wrapped).Value(); }
template <typename T, typename U = typename T::element_type> T __from_variant(__VARIANT_TYPE& wrapped) {
    return std::dynamic_pointer_cast<U>((std::shared_ptr<Instance>)std::get<InstanceRef>(wrapped)); }

class Variant {
    __VARIANT_TYPE wrapped;
public:
    template <typename T> Variant(T obj) : wrapped(__to_variant(obj)) {}
    template <typename T> T get() { return __from_variant<T>(wrapped); }
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