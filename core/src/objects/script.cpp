#include "script.h"
#include "logger.h"
#include "objects/base/instance.h"
#include "objects/base/member.h"
#include "objects/script/scriptcontext.h"
#include "objects/workspace.h"
#include "lua.h"

Script::Script(): Instance(&TYPE) {
    source = "print \"Hello, world!\"";
}

Script::~Script() {
}

void Script::Run() {
    lua_State* L = dataModel().value()->GetService<ScriptContext>()->state;

    // Initialize script globals
    lua_getglobal(L, "_G");
    
    lua_pushstring(L, "game");
    Data::InstanceRef(dataModel().value()).PushLuaValue(L);
    lua_rawset(L, -3);

    lua_pushstring(L, "workspace");
    Data::InstanceRef(dataModel().value()->GetService<Workspace>()).PushLuaValue(L);
    lua_rawset(L, -3);

    lua_pop(L, 1);

    luaL_loadstring(L, source.c_str());
    int status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (status != 0) {
        Logger::error(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

void Script::Stop() {
    // TODO:
}