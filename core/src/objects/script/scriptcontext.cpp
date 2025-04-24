#include "scriptcontext.h"
#include "logger.h"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lualib.h>

static int redirectPrint(lua_State*);
static const struct luaL_Reg luaglobals [] = {
    {"print", redirectPrint},
    {NULL, NULL} /* end of array */
};

const InstanceType ScriptContext::TYPE = {
    .super = &Instance::TYPE,
    .className = "ScriptContext",
    .constructor = &ScriptContext::Create,
    .flags = INSTANCE_NOTCREATABLE | INSTANCE_SERVICE | INSTANCE_HIDDEN,
};

const InstanceType* ScriptContext::GetClass() {
    return &TYPE;
}

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
    luaL_openlibs(state);

    // Override print
    // https://stackoverflow.com/a/4514193/16255372
    
    lua_getglobal(state, "_G");
    luaL_register(state, NULL, luaglobals);
    lua_pop(state, 1);
}

static int redirectPrint(lua_State* L) {
    std::string buf;

    int nargs = lua_gettop(L);

    for (int i=1; i <= nargs; i++) {
        std::string arg(lua_tostring(L, -1));
        lua_pop(L, 1);
        buf += arg;
    }

    Logger::infof(buf);

    return 0;
}