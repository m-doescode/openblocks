#pragma once

#include "../../datatypes/base.h"
#include "datatypes/meta.h"
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <variant>

class Instance;

struct FieldCodec {
    void (*write)(Data::Variant source, void* destination);
    Data::Variant (*read)(void* source);
};

template <typename T, typename U>
void _writeCodec(Data::Variant source, void* destination) {
    *(U*)destination = source.get<T>().value;
}

template <typename T, typename U>
Data::Variant _readCodec(void* source) {
    return T(*(U*)source);
}

template <typename T, typename U>
constexpr FieldCodec fieldCodecOf() {
    return FieldCodec {
        .write = &_writeCodec<T, U>,
        .read = &_readCodec<T, U>,
    };
}

template <typename T>
std::function<void(std::string name)> memberFunctionOf(void(T::*func)(std::string), T* obj) {
    return std::bind(func, obj, std::placeholders::_1);
}

struct PropertyMeta {
    void* backingField;
    const Data::TypeInfo* type;
    FieldCodec codec;
    std::optional<std::function<void(std::string name)>> updateCallback;
};

typedef std::variant<PropertyMeta> MemberMeta;

struct MemberMap {
    std::optional<std::unique_ptr<MemberMap>> super;
    std::map<std::string, PropertyMeta> members;
};

struct MemberNotFound {};