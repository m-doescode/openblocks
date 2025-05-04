#include "scriptcontext.h"
#include "datatypes/meta.h"
#include "logger.h"
#include <cstdint>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lualib.h>

static int g_print(lua_State*);
static int g_require(lua_State*);
static const struct luaL_Reg luaglobals [] = {
    {"print", g_print},
    {"require", g_require},
    {NULL, NULL} /* end of array */
};

std::string unsafe_globals[] = {
    "loadfile", "loadstring", "load", "dofile", "getfenv", "setfenv"
};

ScriptContext::ScriptContext(): Service(&TYPE) {
}

ScriptContext::~ScriptContext() {
    if (state)
        lua_close(state);
}

void ScriptContext::InitService() {
    if (initialized) return;
    initialized = true;

    state = luaL_newstate();
    luaopen_base(state);
    luaopen_math(state);
    luaopen_string(state);
    luaopen_table(state);
    // luaopen_io(state);
    // luaopen_os(state);
    // luaopen_package(state);
    // luaopen_debug(state);
    luaopen_bit(state);

    // TODO: custom os library

    // Override print
    // https://stackoverflow.com/a/4514193/16255372
    
    lua_getglobal(state, "_G");
    luaL_register(state, NULL, luaglobals);

    // Remove misc dangerous functions
    for (std::string key : unsafe_globals) {
        lua_pushstring(state, key.c_str());
        lua_pushnil(state);
        lua_rawset(state, -3);
    }

    lua_pop(state, 1);

}

// https://www.lua.org/source/5.1/lbaselib.c.html
static int g_print(lua_State* L) {
    std::string buf;

    int nargs = lua_gettop(L);

    lua_getglobal(L, "tostring");
    for (int i=1; i <= nargs; i++) {
        lua_pushvalue(L, -1); // push tostring
        lua_pushvalue(L, i); // push current arg
        lua_call(L, 1, 1); // call tostring with current arg (#1 arguments)
        // lua_call automatically pops function and arguments

        const char* str = lua_tostring(L, -1); // convert result into c-string
        lua_pop(L, 1); // pop result
        
        buf += str;
    }

    Logger::info(buf);

    return 0;
}

static int g_require(lua_State* L) {
    int nargs = lua_gettop(L);
    if (nargs < 1) return luaL_error(L, "expected argument module");

    return luaL_error(L, "require is not yet implemented");
}