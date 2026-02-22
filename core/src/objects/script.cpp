#include "script.h"
#include "common.h"
#include "datatypes/variant.h"
#include "lauxlib.h"
#include "logger.h"
#include "objectmodel/property.h"
#include "objectmodel/type.h"
#include "objects/base/instance.h"
#include "objects/base/member.h"
#include "objects/service/script/scriptcontext.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"
#include "datatypes/ref.h"
#include "luaapis.h" // IWYU pragma: keep
#include <algorithm>
#include <memory>

INSTANCE_IMPL(Script)

int script_errhandler(lua_State*);

InstanceType Script::__buildType() {
    return make_instance_type<Script>(
        "Script",
        set_explorer_icon("script"),
        def_property("Source", &Script::source, PROP_HIDDEN)
    );
}

Script::Script() {
    source = "print(\"Hello, world!\")";
}

Script::~Script() {
}

void Script::Run() {
    std::shared_ptr<ScriptContext> scriptContext = dataModel()->GetService<ScriptContext>();

    lua_State* L = scriptContext->state;
    int top = lua_gettop(L);

    // Create thread
    this->thread = lua_newthread(L);
    lua_State* Lt = thread;

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

    // Initialize script globals
    scriptContext->NewEnvironment(Lt); // Pushes envtable, metatable

    // Set script in metatable source
    InstanceRef(shared_from_this()).PushLuaValue(Lt);
    lua_setfield(Lt, -2, "source");

    lua_pop(Lt, 1); // Pop metatable

    // Set script in environment
    InstanceRef(shared_from_this()).PushLuaValue(Lt);
    lua_setfield(Lt, -2, "script");

    lua_setfenv(Lt, -2); // Set env of loaded function

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

static std::shared_ptr<Script> getfsource(lua_State* L, lua_Debug* dbg) {
    int top = lua_gettop(L);

    lua_getinfo(L, "f", dbg);
    lua_getfenv(L, -1); // Get fenv of stack pos
    if (lua_isnil(L, -1)) { // No env could be found
        lua_settop(L, top);
        return nullptr;
    }

    // Get source from metatable
    lua_getmetatable(L, -1);
    lua_getfield(L, -1, "source");

    auto result = InstanceRef::FromLuaValue(L, -1);
    if (!result) {
        lua_settop(L, top);
        return nullptr;
    }

    lua_settop(L, top);

    std::shared_ptr<Instance> ref = result.expect().get<InstanceRef>();
    if (!ref->IsA<Script>()) return nullptr;

    return ref->CastTo<Script>().expect();
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

        // Find script source
        std::shared_ptr<Script> source = getfsource(L, &dbg);

        Logger::scriptLogf("'%s', Line %d", Logger::LogLevel::TRACE, {source, dbg.currentline}, dbg.source, dbg.currentline);
    }

    Logger::trace("Stack end");
    
    return 0;
}