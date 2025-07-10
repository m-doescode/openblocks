#include "objects/base/instance.h"
#include "datatypes/ref.h"
#include "luaapis.h" // IWYU pragma: keep
#include "objects/meta.h"
#include <memory>

static int lib_Instance_index(lua_State*);
static int lib_Instance_tostring(lua_State*);
static const struct luaL_Reg lib_Instance_metatable [] = {
    {"__index", lib_Instance_index},
    {"__tostring", lib_Instance_tostring},
    {NULL, NULL} /* end of array */
};

static int __lua_impl__Instance__new(lua_State* L) {
    int n = lua_gettop(L);

    // First argument (className)
    std::string className = luaL_checkstring(L, 1);

    // [Optional] Second argument (parent)
    std::shared_ptr<Instance> parent;
    if (n > 1) {
        parent = **(std::shared_ptr<Instance>**)luaL_checkudata(L, 2, "__mt_instance");
    }

    // Look up class name
    if (INSTANCE_MAP.count(className) == 0)
        return luaL_error(L, "Attempt to create instance of unknown type '%s'", className.c_str());

    const InstanceType* type = INSTANCE_MAP[className];

    if (type->flags & (INSTANCE_NOTCREATABLE | INSTANCE_SERVICE) || type->constructor == nullptr)
        return luaL_error(L, "Attempt to create Instance of type '%s', which is not creatable", className.c_str());

    std::shared_ptr<Instance> object = type->constructor();
    
    if (parent != nullptr)
        object->SetParent(parent);

    InstanceRef(object).PushLuaValue(L);
    return 1;
}

void Instance::PushLuaLibrary(lua_State* L) {
    lua_getglobal(L, "_G");
    lua_pushstring(L, "Instance");

    lua_newuserdata(L, 0);

    // Create the library's metatable
    luaL_newmetatable(L, "__mt_lib_Instance");
    luaL_register(L, NULL, lib_Instance_metatable);
    lua_setmetatable(L, -2);

    lua_rawset(L, -3);
    lua_pop(L, 1);
}


int lib_Instance_tostring(lua_State* L) {
    lua_pushstring(L, "Instance");
    return 1;
}

static int lib_Instance_index(lua_State* L) {
    std::string key(lua_tostring(L, 2));
    lua_pop(L, 2);

    if (key == "new") {
        lua_pushcfunction(L, __lua_impl__Instance__new);
        return 1;
    }

    return luaL_error(L, "%s is not a valid member of %s\n", key.c_str(), "Instance");
}

