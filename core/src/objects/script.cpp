#include "script.h"
#include "logger.h"
#include "objects/base/instance.h"
#include "objects/base/member.h"
#include "objects/script/scriptcontext.h"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>

const InstanceType Script::TYPE = {
    .super = &Instance::TYPE,
    .className = "Script",
    .constructor = &Script::Create,
    .explorerIcon = "script",
};

const InstanceType* Script::GetClass() {
    return &TYPE;
}

Script::Script(): Instance(&TYPE) {
    this->memberMap = std::make_unique<MemberMap>(MemberMap {
        .super = std::move(this->memberMap),
        .members = {
            { "Source", {
                .backingField = &source,
                .type = &Data::String::TYPE,
                .codec = fieldCodecOf<Data::String, std::string>(),
                .flags = PROP_HIDDEN,
            }},
        }
    });

    source = "print \"Hello, world!\"";
}

Script::~Script() {
}

void Script::Run() {
    lua_State* L = dataModel().value()->GetService<ScriptContext>()->state;

    // Initialize script globals
    

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