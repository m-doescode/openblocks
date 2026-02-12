#include "enum/meta.h"
#include "globals.h"
#include "luaapis.h" // IWYU pragma: keep
#include <lua.h>

// Enums

int Enums_getEnums(lua_State*);

int Enums_index(lua_State*);
int Enums_tostring(lua_State*);

static const luaL_Reg lib_Enums[] = {
    { "__index", Enums_index },
    { "__newindex", X_LUA_NOTMODIFIABLE("Enums") },
    { "__tostring", Enums_tostring },
    { NULL, NULL }
};

// Main

void push_enum_global(lua_State* L) {
    lua_getglobal(L, "_G");

    // New userdata
    lua_newuserdata(L, 0);

    // Create the library's metatable
    bool notexists = luaL_newmetatable(L, "Enums");
    if (notexists) luaL_register(L, NULL, lib_Enums);
    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "Enum");
    lua_pop(L, 1);
}

// ======= IMPLEMENTATION =======

// Enums

int Enums_getEnums(lua_State* L) {
    lua_createtable(L, ENUM_MAP.size(), 0);

    for (auto& [key, enumObj] : ENUM_MAP) {
        enumObj->PushLuaValue(L);
        lua_rawseti(L, 2, lua_objlen(L, 2)+1);
    }

    return 1;
}

int Enums_index(lua_State* L) {
    luaL_checkstring(L, 2);
    std::string key = lua_tostring(L, 2);

    if (key == "GetEnums") {
        lua_pushcclosure(L, Enums_getEnums, 0);
        return 1;
    }

    if (ENUM_MAP.count(key) == 0) {
        return luaL_error(L, "%s is not a valid EnumItem", key.c_str());
    }
    
    const Enum* enumObj = ENUM_MAP[key];
    enumObj->PushLuaValue(L);
    return 1;
}

int Enums_tostring(lua_State* L) {
    lua_pushstring(L, "Enums");
    return 1;
}