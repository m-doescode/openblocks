#include "datatypes/ref.h"
#include "datatypes/base.h"
#include "error/data.h"
#include "logger.h"
#include "meta.h" // IWYU pragma: keep
#include <memory>
#include <optional>
#include "objects/base/instance.h"
#include "lua.h"
#include "objects/base/member.h"

Data::InstanceRef::InstanceRef() {};
Data::InstanceRef::InstanceRef(std::weak_ptr<Instance> instance) : ref(instance) {};
Data::InstanceRef::~InstanceRef() = default;

const Data::TypeInfo Data::InstanceRef::TYPE = {
    .name = "Ref",
    .deserializer = &Data::InstanceRef::Deserialize,
    .fromLuaValue = &Data::InstanceRef::FromLuaValue,
};

const Data::TypeInfo& Data::InstanceRef::GetType() const { return Data::InstanceRef::TYPE; };

const Data::String Data::InstanceRef::ToString() const {
    return ref.expired() ? "" : ref.lock()->name;
}

Data::InstanceRef::operator std::weak_ptr<Instance>() {
    return ref;
}

// Serialization

void Data::InstanceRef::Serialize(pugi::xml_node node) const {
    // node.text().set(this->ToHex());
}

Data::Variant Data::InstanceRef::Deserialize(pugi::xml_node node) {
    return Data::InstanceRef();
}

static int inst_gc(lua_State*);
static int inst_index(lua_State*);
static int inst_newindex(lua_State*);
static const struct luaL_Reg metatable [] = {
    {"__gc", inst_gc},
    {"__index", inst_index},
    {"__newindex", inst_newindex},
    {NULL, NULL} /* end of array */
};

void Data::InstanceRef::PushLuaValue(lua_State* L) const {
    if (ref.expired()) return lua_pushnil(L);

    int n = lua_gettop(L);

    auto userdata = (std::shared_ptr<Instance>**)lua_newuserdata(L, sizeof(void*));

    // Create new pointer, and assign userdata a pointer to it
    std::shared_ptr<Instance>* ptr = new std::shared_ptr<Instance>(ref);
    *userdata = ptr;

    // Create the instance's metatable
    luaL_newmetatable(L, "__mt_instance");
    luaL_register(L, NULL, metatable);

    lua_setmetatable(L, n+1);
}

result<Data::Variant, LuaCastError> Data::InstanceRef::FromLuaValue(lua_State* L, int idx) {
    if (lua_isnil(L, idx))
        return Data::Variant(Data::InstanceRef());
    if (!lua_isuserdata(L, idx))
        return LuaCastError(lua_typename(L, idx), "Instance");
    // TODO: overhaul this to support other types
    auto userdata = (std::shared_ptr<Instance>**)lua_touserdata(L, idx);
    return Data::Variant(Data::InstanceRef(**userdata));
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
        Data::Variant value = inst->GetPropertyValue(key).expect();
        value.PushLuaValue(L);
        return 1;
    }

    // Look for child
    std::optional<std::shared_ptr<Instance>> child = inst->FindFirstChild(key);
    if (child) {
        Data::InstanceRef(child.value()).PushLuaValue(L);
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
        return luaL_error(L, "Cannot set property Parent (%s) of %s, parent is locked", inst->GetParent() ? inst->GetParent().value()->name.c_str() : "NULL", inst->GetClass()->className.c_str());

    result<Data::Variant, LuaCastError> value = meta->type->fromLuaValue(L, -1);
    lua_pop(L, 3);

    if (value.isError())
        return luaL_error(L, "%s", value.errorMessage().value().c_str());
    inst->SetPropertyValue(key, value.expect()).expect();
    return 0;
}