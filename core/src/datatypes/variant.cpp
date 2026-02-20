#include "variant.h"
#include "datatypes/base.h"
#include "datatypes/cframe.h"
#include "datatypes/enum.h"
#include "datatypes/primitives.h"
#include "datatypes/ref.h"
#include "datatypes/signal.h"
#include "datatypes/vector.h"
#include "error/data.h"
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

const TypeDesc* VARIANT_TYPES[] {
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
    &Enum::TYPE,
    &EnumItem::TYPE,
    &ARRAY_TYPE
};

const TypeMeta Variant::GetTypeMeta() const {
    return VARIANT_TYPES[wrapped.index()];
}

const TypeDesc* Variant::GetType() const {
    return VARIANT_TYPES[wrapped.index()];
}

std::string Variant::ToString() const {
    if (!VARIANT_TYPES[wrapped.index()]->pushLuaValue) {
        Logger::fatalErrorf("Data type %s does not implement toString", VARIANT_TYPES[wrapped.index()]->name.c_str());
    }
    return VARIANT_TYPES[wrapped.index()]->toString(*this);
}

void Variant::Serialize(pugi::xml_node node) const {
    if (!VARIANT_TYPES[wrapped.index()]->pushLuaValue) {
        Logger::fatalErrorf("Data type %s does not implement serializer", VARIANT_TYPES[wrapped.index()]->name.c_str());
    }
    VARIANT_TYPES[wrapped.index()]->serialize(*this, node);
}

void Variant::PushLuaValue(lua_State* state) const {
    if (!VARIANT_TYPES[wrapped.index()]->pushLuaValue) {
        Logger::fatalErrorf("Data type %s does not implement pushLuaValue", VARIANT_TYPES[wrapped.index()]->name.c_str());
    }
    VARIANT_TYPES[wrapped.index()]->pushLuaValue(*this, state);
}

result<Variant, DataParseError> Variant::Deserialize(pugi::xml_node node, const TypeMeta type) {
    if (!type.descriptor->deserialize) {
        Logger::fatalErrorf("Data type %s does not implement deserialize", type.descriptor->name.c_str());
        return DataParseError(node.text().as_string(), type.descriptor->name);
    }
    return type.descriptor->deserialize(node, type);
}

std::map<std::string, const TypeDesc*> TYPE_MAP = {
    { "null", &NULL_TYPE },
    { "bool", &BOOL_TYPE },
    { "int", &INT_TYPE },
    { "float", &FLOAT_TYPE },
    { "string", &STRING_TYPE },
    { "Vector3", &Vector3::TYPE },
    { "CoordinateFrame", &CFrame::TYPE },
    { "Color3", &Color3::TYPE },
    { "Ref", &InstanceRef::TYPE },
    { "token", &EnumItem::TYPE },
};