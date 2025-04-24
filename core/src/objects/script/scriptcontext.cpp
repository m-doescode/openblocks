#include "scriptcontext.h"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>

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
}