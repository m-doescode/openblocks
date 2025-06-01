#include "variant.h"
#include "datatypes/base.h"
#include "datatypes/cframe.h"
#include "datatypes/primitives.h"
#include "datatypes/ref.h"
#include "datatypes/signal.h"
#include "datatypes/vector.h"
#include "logger.h"
#include "panic.h"
#include <pugixml.hpp>
#include <string>
#include <variant>

[[noreturn]] inline void unreachable() {
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

const TypeDescriptor* VARIANT_TYPES[] {
    &NULL_TYPE,
    &BOOL_TYPE,
    &INT_TYPE,
    &FLOAT_TYPE,
    &STRING_TYPE,
    &Vector3::TYPE,
    &CFrame::TYPE,
    &Color3::TYPE,
    &InstanceRef::TYPE,
    &SignalRef::TYPE,
    &SignalConnectionRef::TYPE,
};

std::string Variant::ToString() const {
    return VARIANT_TYPES[wrapped.index()]->toString(*this);
}

void Variant::Serialize(pugi::xml_node node) const {
    VARIANT_TYPES[wrapped.index()]->serializer(*this, node);
}

void Variant::PushLuaValue(lua_State* state) const {
    printf("What %zu\n", wrapped.index());
    VARIANT_TYPES[wrapped.index()]->pushLuaValue(*this, state);
}

Variant Variant::Deserialize(pugi::xml_node node, const TypeInfo type) {
    if (type.type == DATA_VALUE)
        return type.descriptor->deserializer(node);
    else
        panic(); //TODO: NYI
}

std::map<std::string, const TypeDescriptor*> TYPE_MAP = {
    { "null", &NULL_TYPE },
    { "bool", &BOOL_TYPE },
    { "int", &INT_TYPE },
    { "float", &FLOAT_TYPE },
    { "string", &STRING_TYPE },
    { "Vector3", &Vector3::TYPE },
    { "CoordinateFrame", &CFrame::TYPE },
    { "Color3", &Color3::TYPE },
    { "Ref", &InstanceRef::TYPE },
};