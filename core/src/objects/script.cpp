#include "script.h"
#include "logger.h"
#include "objects/base/instance.h"
#include "objects/base/member.h"
#include "objects/script/scriptcontext.h"
#include "objects/workspace.h"
#include "objects/datamodel.h"
#include "datatypes/ref.h"
#include "lua.h"
#include <luajit-2.1/lua.h>

Script::Script(): Instance(&TYPE) {
    source = "print \"Hello, world!\"";
}

Script::~Script() {
}

void Script::Run() {
    lua_State* L = dataModel().value()->GetService<ScriptContext>()->state;

    // Create thread
    this->thread = lua_newthread(L);
    lua_State* Lt = thread;

    // Initialize script globals
    lua_getglobal(Lt, "_G");
    
    lua_pushstring(Lt, "game");
    Data::InstanceRef(dataModel().value()).PushLuaValue(Lt);
    lua_rawset(Lt, -3);

    lua_pushstring(Lt, "workspace");
    Data::InstanceRef(dataModel().value()->GetService<Workspace>()).PushLuaValue(Lt);
    lua_rawset(Lt, -3);

    lua_pop(Lt, 1); // _G

    // Load source and push onto thread stack as function ptr
    luaL_loadstring(Lt, source.c_str());
    
    int status = lua_resume(Lt, 0);
    if (status > LUA_YIELD) {
        Logger::error(lua_tostring(Lt, -1));
        lua_pop(Lt, 1); // Pop return value
    }

    lua_pop(L, 1); // Pop the thread
}

void Script::Stop() {
    // TODO:
}