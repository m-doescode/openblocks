#include "script.h"
#include "common.h"
#include "logger.h"
#include "objects/base/instance.h"
#include "objects/base/member.h"
#include "objects/script/scriptcontext.h"
#include "objects/workspace.h"
#include "objects/datamodel.h"
#include "datatypes/ref.h"
#include "lua.h"
#include <algorithm>
#include <memory>

int script_wait(lua_State*);
int script_delay(lua_State*);

Script::Script(): Instance(&TYPE) {
    source = "print(\"Hello, world!\")";
}

Script::~Script() {
}

void Script::Run() {
    std::shared_ptr<ScriptContext> scriptContext = dataModel().value()->GetService<ScriptContext>();

    lua_State* L = scriptContext->state;

    // Create thread
    this->thread = lua_newthread(L);
    lua_State* Lt = thread;

    // Initialize script globals
    lua_getglobal(Lt, "_G");
    
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

    // Load source and push onto thread stack as function ptr
    // luaL_loadstring(Lt, source.c_str());
    luaL_loadbuffer(Lt, source.c_str(), source.size(), scriptContext->RegisterScriptSource(shared<Script>()).c_str());
    
    int status = lua_resume(Lt, 0);
    if (status > LUA_YIELD) {
        lua_Debug dbg;
        lua_getstack(Lt, 1, &dbg);
        lua_getinfo(Lt, "S", &dbg);

        std::weak_ptr<Script> source = scriptContext->GetScriptFromSource(dbg.source);

        std::string errorMessage = lua_tostring(Lt, -1);
        if (!source.expired())
            errorMessage = source.lock()->GetFullName() + errorMessage.substr(errorMessage.find(':'));

        Logger::error(errorMessage);
        lua_pop(Lt, 1); // Pop return value

        Logger::traceStart();

        int stack = 1;
        while (lua_getstack(Lt, stack++, &dbg)) {
            lua_getinfo(Lt, "nlSu", &dbg);
            
            std::weak_ptr<Script> source = scriptContext->GetScriptFromSource(dbg.source);
            if (source.expired()) {
                Logger::trace(dbg.source, dbg.currentline);
            } else {
                Logger::trace(source.lock()->GetFullName(), dbg.currentline, &source);
            }
        }

        Logger::traceEnd();
    }

    lua_pop(L, 1); // Pop the thread
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