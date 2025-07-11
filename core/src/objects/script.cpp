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