#include "scriptcontext.h"
#include "datatypes/cframe.h"
#include "datatypes/color3.h"
#include "datatypes/ref.h"
#include "datatypes/vector.h"
#include "logger.h"
#include "objects/datamodel.h"
#include "objects/service/workspace.h"
#include "timeutil.h"
#include <chrono>
#include <ctime>
#include <string>
#include "luaapis.h" // IWYU pragma: keep

const char* WRAPPER_SRC = "local func, errhandler = ... return function(...) local args = {...} xpcall(function() func(unpack(args)) end, errhandler) end";

int g_wait(lua_State*);
int g_delay(lua_State*);
int g_tick(lua_State*);
static int g_print(lua_State*);
static int g_require(lua_State*);
static const luaL_Reg luaglobals [] = {
    {"print", g_print},
    {"require", g_require},
    {NULL, NULL} /* end of array */
};

std::string unsafe_globals[] = {
    // Todo implement our own "safe" setfenv/getfenv
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

    Vector3::PushLuaLibrary(state);
    CFrame::PushLuaLibrary(state);
    Color3::PushLuaLibrary(state);
    Instance::PushLuaLibrary(state);
    
    // Push reference to datamodel
    lua_pushlightuserdata(state, &*dataModel());
    lua_setfield(state, LUA_REGISTRYINDEX, "dataModel");

    // Add other globals
    lua_getglobal(state, "_G");

    InstanceRef(dataModel()).PushLuaValue(state);
    lua_setfield(state, -2, "game");

    InstanceRef(dataModel()->GetService<Workspace>()).PushLuaValue(state);
    lua_setfield(state, -2, "workspace");

    lua_pushlightuserdata(state, this);
    lua_pushcclosure(state, g_wait, 1);
    lua_setfield(state, -2, "wait");

    lua_pushlightuserdata(state, this);
    lua_pushcclosure(state, g_delay, 1);
    lua_setfield(state, -2, "delay");

    lua_pushcclosure(state, g_tick, 0);
    lua_setfield(state, -2, "tick");

    lua_pop(state, 1); // _G

    // Add wrapper function
    luaL_loadbuffer(state, WRAPPER_SRC, strlen(WRAPPER_SRC), "=PCALL_WRAPPER");
    lua_setfield(state, LUA_REGISTRYINDEX, "LuaPCallWrapper");

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

    lua_pop(state, 1); // _G

    lua_pushlightuserdata(state, &sleepingThreads);
    lua_newtable(state);
    lua_settable(state, LUA_REGISTRYINDEX);
}

void ScriptContext::PushThreadSleep(lua_State* thread, float delay) {
    // A thread is allowed to sleep multiple times at once, though this is a very edge-case scenario

    SleepingThread sleep;
    sleep.thread = thread;
    sleep.timeYieldedWhen = tu_clock_micros();
    sleep.targetTimeMicros = tu_clock_micros() + delay * 1'000'000;

    sleepingThreads.push_back(sleep);

    // Add to registry so it doesn't get GC'd

    // https://stackoverflow.com/a/17138663/16255372
    lua_pushlightuserdata(state, &sleepingThreads);
    lua_gettable(state, LUA_REGISTRYINDEX);

    lua_pushthread(thread); // key
    lua_xmove(thread, state, 1);
    lua_pushboolean(state, true); // value
    lua_rawset(state, -3); // set

    lua_pop(state, 1); // pop sleepingThreads
}

tu_time_t schedTime;
void ScriptContext::RunSleepingThreads() {
    tu_time_t startTime = tu_clock_micros();
    size_t i;
    for (i = 0; i < sleepingThreads.size();) {
        bool deleted = false;

        // TODO: Remove threads that belong to non-existent scripts
        SleepingThread sleep = sleepingThreads[i];
        if (tu_clock_micros() >= sleep.targetTimeMicros) {
            // Time args
            lua_pushnumber(sleep.thread, float(tu_clock_micros() - sleep.timeYieldedWhen) / 1'000'000);
            lua_pushnumber(sleep.thread, float(tu_clock_micros()) / 1'000'000);
            lua_resume(sleep.thread, 2);

            // Remove thread
            deleted = true;
            sleepingThreads.erase(sleepingThreads.begin() + i);

            // Erase from registry
            lua_pushlightuserdata(state, &sleepingThreads);
            lua_gettable(state, LUA_REGISTRYINDEX);
            
            lua_pushthread(sleep.thread); // key
            lua_xmove(sleep.thread, state, 1);
            lua_pushnil(state);
            lua_rawset(state, -3); // set

            lua_pop(state, 1); // sleepingThreads
        }

        if (!deleted)
            i++;
    }
    if (i > 0)
        schedTime = tu_clock_micros() - startTime;
}

// Temporary stopgap until RunSleepingThreads can clear threads that belong to
// scripts no longer parented to the DataModel
void ScriptContext::DebugClearSleepingThreads() {
    sleepingThreads.clear();
}

void ScriptContext::NewEnvironment(lua_State* L) {
    lua_newtable(L); // Env table
    lua_newtable(L); // Metatable

    // Push __index
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_setfield(L, -2, "__index");

    // Push __metatable
    lua_pushstring(L, "metatable is locked");
    lua_setfield(L, -2, "__metatable");

    // Copy metatable and set the env table's metatable
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -3);

    // Remainder on stack:
    // 1. Env table
    // 2. Metatable
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
        
        if (i > 1) buf += '\t';
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

int g_wait(lua_State* L) {
    ScriptContext* scriptContext = (ScriptContext*)lua_touserdata(L, lua_upvalueindex(1));
    float secs = lua_gettop(L) == 0 ? 0.03 : std::max(luaL_checknumber(L, 1), 0.03);
    if (lua_gettop(L) > 0) lua_pop(L, 1); // pop secs

    scriptContext->PushThreadSleep(L, secs);

    // Yield
    return lua_yield(L, 0);
}

int g_delay(lua_State* L) {
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

int g_tick(lua_State* L) {
    std::chrono::time_point now_local = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(now_local.time_since_epoch());
    uint64_t _10millis = us.count() / 100;
    double secs = (double)_10millis / 10000;

    lua_pushnumber(L, secs);
    return 1;
}