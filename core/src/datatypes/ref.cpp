#include "datatypes/ref.h"
#include "datatypes/base.h"
#include "error/data.h"
#include "logger.h"
#include "variant.h" // IWYU pragma: keep
#include <memory>
#include <optional>
#include "objects/base/instance.h"
#include "luaapis.h" // IWYU pragma: keep
#include "objects/base/member.h"
#include <pugixml.hpp>

TypeMeta::TypeMeta(const InstanceType* instType) : descriptor(&InstanceRef::TYPE), instType(instType) {}

InstanceRef::InstanceRef() : ref(nullptr) {};
InstanceRef::InstanceRef(std::weak_ptr<Instance> instance) : ref(instance.expired() ? nullptr : instance.lock()) {};
InstanceRef::~InstanceRef() = default;

const TypeDesc InstanceRef::TYPE = {
    .name = "Ref",
    .serialize = toVariantFunction(&InstanceRef::Serialize),
    .deserialize = toVariantGeneratorNoMeta(&InstanceRef::Deserialize),
    .toString = toVariantFunction(&InstanceRef::ToString),
    .fromString = nullptr,
    .pushLuaValue = toVariantFunction(&InstanceRef::PushLuaValue),
    .fromLuaValue = &InstanceRef::FromLuaValue,
};

const std::string InstanceRef::ToString() const {
    return ref == nullptr ? "NULL" : ref->name;
}

InstanceRef::operator std::shared_ptr<Instance>() {
    return ref;
}

InstanceRef::operator std::weak_ptr<Instance>() {
    return ref;
}

bool InstanceRef::operator ==(InstanceRef other) const {
    return this->ref == other.ref;
}

// Serialization

void InstanceRef::Serialize(pugi::xml_node node) const {
    // Handled by Instance
    panic();
}

result<InstanceRef, DataParseError> InstanceRef::Deserialize(pugi::xml_node node) {
    // Handled by Instance
    panic();
}

static int inst_gc(lua_State*);
static int inst_index(lua_State*);
static int inst_newindex(lua_State*);
static int inst_tostring(lua_State*);
static int inst_eq(lua_State*);
static const struct luaL_Reg metatable [] = {
    {"__gc", inst_gc},
    {"__index", inst_index},
    {"__newindex", inst_newindex},
    {"__tostring", inst_tostring},
    {"__eq", inst_eq},
    {NULL, NULL} /* end of array */
};

void InstanceRef::PushLuaValue(lua_State* L) const {
    if (ref == nullptr) return lua_pushnil(L);

    // Get or create InstanceRef table
    lua_getfield(L, LUA_REGISTRYINDEX, "__instances");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        
        // Set metatable
        lua_newtable(L);
        lua_pushstring(L, "kv");
        lua_setfield(L, -2, "__mode");
        lua_setmetatable(L, -2);

        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, "__instances");
    }

    // Check if value already exists, and if so, return that instead
    lua_pushlightuserdata(L, ref.get());
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_remove(L, -2); // Remove __instances
        return;
    }
    lua_pop(L, 1);

    int n = lua_gettop(L);

    auto userdata = (std::shared_ptr<Instance>**)lua_newuserdata(L, sizeof(void*));

    // Create new pointer, and assign userdata a pointer to it
    std::shared_ptr<Instance>* ptr = new std::shared_ptr<Instance>(ref);
    *userdata = ptr;

    // Create the instance's metatable
    luaL_newmetatable(L, "__mt_instance");
    luaL_register(L, NULL, metatable);
    lua_setmetatable(L, n+1);

    // Add instance to __instances
    lua_pushlightuserdata(L, ref.get());
    lua_pushvalue(L, -2); // Push userdata
    lua_rawset(L, -4); // Put into __instance
    lua_remove(L, -2); // Remove __instance
}

result<Variant, LuaCastError> InstanceRef::FromLuaValue(lua_State* L, int idx) {
    if (lua_isnil(L, idx))
        return Variant(InstanceRef());
    if (!lua_isuserdata(L, idx))
        return LuaCastError(lua_typename(L, idx), "Instance");
    // TODO: overhaul this to support other types
    auto userdata = (std::shared_ptr<Instance>**)lua_touserdata(L, idx);
    return Variant(InstanceRef(**userdata));
}

static int inst_gc(lua_State* L) {
    // Destroy the contained shared_ptr
    auto userdata = (std::shared_ptr<Instance>**)lua_touserdata(L, -1);
    delete *userdata;
    lua_pop(L, 1);

    return 0;
}

// __index(t,k)
static int inst_index(lua_State* L) {
    auto userdata = (std::shared_ptr<Instance>**)lua_touserdata(L, 1);
    std::shared_ptr<Instance> inst = **userdata;
    std::string key(lua_tostring(L, 2));
    lua_pop(L, 2);
    
    // Read property
    std::optional<PropertyMeta> meta = inst->GetPropertyMeta(key);
    if (meta) {
        Variant value = inst->GetProperty(key).expect();
        value.PushLuaValue(L);
        return 1;
    }

    // Look for child
    nullable std::shared_ptr<Instance> child = inst->FindFirstChild(key);
    if (child) {
        InstanceRef(child).PushLuaValue(L);
        return 1;
    }

    return luaL_error(L, "'%s' is not a valid member of %s", key.c_str(), inst->GetClass()->className.c_str());
}

// __newindex(t,k,v)
static int inst_newindex(lua_State* L) {
    auto userdata = (std::shared_ptr<Instance>**)lua_touserdata(L, 1);
    std::shared_ptr<Instance> inst = **userdata;
    std::string key(lua_tostring(L, 2));
    
    // Validate property
    std::optional<PropertyMeta> meta = inst->GetPropertyMeta(key);
    if (!meta)
        return luaL_error(L, "'%s' is not a valid member of %s", key.c_str(), inst->GetClass()->className.c_str());
    if (meta->flags & PROP_READONLY)
        return luaL_error(L, "'%s' of %s is read-only", key.c_str(), inst->GetClass()->className.c_str());
    if (key == "Parent" && inst->IsParentLocked())
        return luaL_error(L, "Cannot set property Parent (%s) of %s, parent is locked", inst->GetParent() ? inst->GetParent()->name.c_str() : "NULL", inst->GetClass()->className.c_str());

    // TODO: Make this work for enums, this is not a solution!!
    result<Variant, LuaCastError> value = meta->type.descriptor->fromLuaValue(L, -1);
    lua_pop(L, 3);

    if (value.isError())
        return luaL_error(L, "%s", value.errorMessage().value().c_str());
    inst->SetProperty(key, value.expect()).expect();
    return 0;
}

static int inst_tostring(lua_State* L) {
    auto userdata = (std::shared_ptr<Instance>**)lua_touserdata(L, 1);
    std::shared_ptr<Instance> inst = **userdata;

    lua_pushstring(L, inst->name.c_str());

    return 1;
}

static int inst_eq(lua_State* L) {
    auto userdata = (std::shared_ptr<Instance>**)lua_touserdata(L, 1);
    std::shared_ptr<Instance> inst = **userdata;
    auto userdata2 = (std::shared_ptr<Instance>**)luaL_checkudata(L, 2, "__mt_instance");
    std::shared_ptr<Instance> inst2 = **userdata2;

    lua_pushboolean(L, inst == inst2);
    return 1;
}
