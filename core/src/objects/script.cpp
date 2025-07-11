#include "script.h"
#include "common.h"
#include "lauxlib.h"
#include "logger.h"
#include "objects/base/instance.h"
#include "objects/base/member.h"
#include "objects/service/script/scriptcontext.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"
#include "datatypes/ref.h"
#include "luaapis.h" // IWYU pragma: keep
#include <algorithm>
#include <memory>

int script_wait(lua_State*);
int script_delay(lua_State*);
int script_errhandler(lua_State*);

Script::Script(): Instance(&TYPE) {
    source = "print(\"Hello, world!\")";
}

Script::~Script() {
}

void Script::Run() {
    std::shared_ptr<ScriptContext> scriptContext = dataModel().value()->GetService<ScriptContext>();

    lua_State* L = scriptContext->state;
    int top = lua_gettop(L);

    // Create thread
    this->thread = lua_newthread(L);
    lua_State* Lt = thread;

    // Initialize script globals
    lua_getglobal(Lt, "_G");

    InstanceRef(shared_from_this()).PushLuaValue(Lt);
    lua_setfield(Lt, -2, "script");
    
    InstanceRef(dataModel().value()).PushLuaValue(Lt);
    lua_setfield(Lt, -2, "game");

    InstanceRef(dataModel().value()->GetService<Workspace>()).PushLuaValue(Lt);
    lua_setfield(Lt, -2, "workspace");

    lua_pushlightuserdata(Lt, scriptContext.get());
    lua_pushcclosure(Lt, script_wait, 1);
    lua_setfield(Lt, -2, "wait");

    lua_pushlightuserdata(Lt, scriptContext.get());
    lua_pushcclosure(Lt, script_delay, 1);
    lua_setfield(Lt, -2, "delay");

    lua_pop(Lt, 1); // _G

    // Push wrapper as thread function
    lua_getfield(Lt, LUA_REGISTRYINDEX, "LuaPCallWrapper");

    // Load source code and push onto thread as upvalue for wrapper
    int status = luaL_loadbuffer(Lt, source.c_str(), source.size(), this->GetFullName().c_str());
    if (status != LUA_OK) {
        // Failed to parse/load chunk
        Logger::error(lua_tostring(Lt, -1));

        lua_settop(L, top);
        return;
    }

    // Push our error handler and then generate the wrapped function
    lua_pushcfunction(Lt, script_errhandler);
    lua_call(Lt, 2, 1);

    // Resume the thread
    lua_resume(Lt, 0);

    lua_pop(L, 1); // Pop the thread
    lua_settop(L, top);
}

void Script::Stop() {
    // TODO:
}

int script_wait(lua_State* L) {
    ScriptContext* scriptContext = (ScriptContext*)lua_touserdata(L, lua_upvalueindex(1));
    float secs = lua_gettop(L) == 0 ? 0.03 : std::max(luaL_checknumber(L, 1), 0.03);
    if (lua_gettop(L) > 0) lua_pop(L, 1); // pop secs

    scriptContext->PushThreadSleep(L, secs);

    // Yield
    return lua_yield(L, 0);
}

int script_delay(lua_State* L) {
    ScriptContext* scriptContext = (ScriptContext*)lua_touserdata(L, lua_upvalueindex(1));
    float secs = std::max(luaL_checknumber(L, 1), 0.03);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    lua_State* Lt = lua_newthread(L); // Create a new thread
    // I think this is memory abuse??
    // Wouldn't popping the thread in this case make it eligible for garbage collection?
    lua_pop(L, 1); // pop the newly created thread so that xmove moves func instead of it into itself
    lua_xmove(L, Lt, 1); // move func
    lua_pop(L, 1); // pop secs

    // Schedule next run
    scriptContext->PushThreadSleep(Lt, secs);

    return 0;
}

int script_errhandler(lua_State* L) {
    std::string errorMessage = lua_tostring(L, -1);
    Logger::error(errorMessage);

    // Traceback

    Logger::trace("Stack start");

    lua_Debug dbg;
    int stack = 1;
    while (lua_getstack(L, stack++, &dbg)) {
        lua_getinfo(L, "nlSu", &dbg);
        // Ignore C frames and internal wrappers
        if (strcmp(dbg.what, "C") == 0 || strcmp(dbg.source, "=PCALL_WRAPPER") == 0)
            continue;

        Logger::scriptLogf("'%s', Line %d", Logger::LogLevel::TRACE, {}, dbg.source, dbg.currentline);
    }

    Logger::trace("Stack end");
    
    return 0;
}