#pragma once

#include "datatypes/base.h"
#include "datatypes/primitives.h"
#include "enum/part.h"
#include "enum/surface.h"
#include <memory>
#include <string>
#include <type_traits>

template <typename T>
struct type_meta_of_t {
    TypeMeta value = &T::TYPE;
};

template <typename T>
struct type_meta_of_t<std::shared_ptr<T>> {
    TypeMeta value = &T::Type();
};

template <typename T>
struct type_meta_of_t<std::weak_ptr<T>> {
    TypeMeta value = &T::Type();
};

template <>
struct type_meta_of_t<std::monostate> {
    TypeMeta value = &NULL_TYPE;
};

template <>
struct type_meta_of_t<bool> {
    TypeMeta value = &BOOL_TYPE;
};

template <>
struct type_meta_of_t<int> {
    TypeMeta value = &INT_TYPE;
};

template <>
struct type_meta_of_t<float> {
    TypeMeta value = &FLOAT_TYPE;
};

template <>
struct type_meta_of_t<std::string> {
    TypeMeta value = &STRING_TYPE;
};

// Until enums get overhauled, we're just going to have to manually specify these
template <>
struct type_meta_of_t<SurfaceType> {
    TypeMeta value = &EnumType::SurfaceType;
};

template <>
struct type_meta_of_t<NormalId> {
    TypeMeta value = &EnumType::NormalId;
};

template <>
struct type_meta_of_t<PartType> {
    TypeMeta value = &EnumType::PartType;
};

template <typename T>
TypeMeta type_meta_of() {
    return type_meta_of_t<T>().value;
}