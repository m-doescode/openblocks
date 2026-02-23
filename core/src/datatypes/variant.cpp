#include "variant.h"
#include "datatypes/base.h"
#include "datatypes/meta.h"
#include "error/data.h"
#include "logger.h"
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

const TypeMeta Variant::GetTypeMeta() const {
    return std::visit([](auto&& value) { return type_meta_of<decltype(value)>(); }, wrapped);
}

const TypeDesc* Variant::GetType() const {
    return GetTypeMeta().descriptor;
}

std::string Variant::ToString() const {
    if (!GetType()->pushLuaValue) {
        Logger::fatalErrorf("Data type %s does not implement toString", GetType()->name.c_str());
    }
    return GetType()->toString(*this);
}

void Variant::Serialize(pugi::xml_node node) const {
    if (!GetType()->pushLuaValue) {
        Logger::fatalErrorf("Data type %s does not implement serializer", GetType()->name.c_str());
    }
    GetType()->serialize(*this, node);
}

void Variant::PushLuaValue(lua_State* state) const {
    if (!GetType()->pushLuaValue) {
        Logger::fatalErrorf("Data type %s does not implement pushLuaValue", GetType()->name.c_str());
    }
    GetType()->pushLuaValue(*this, state);
}

result<Variant, DataParseError> Variant::Deserialize(pugi::xml_node node, const TypeMeta type) {
    if (!type.descriptor->deserialize) {
        Logger::fatalErrorf("Data type %s does not implement deserialize", type.descriptor->name.c_str());
        return DataParseError(node.text().as_string(), type.descriptor->name);
    }
    return type.descriptor->deserialize(node, type);
}