#include "scriptcontext.h"
#include "datatypes/cframe.h"
#include "datatypes/color3.h"
#include "datatypes/vector.h"
#include "logger.h"
#include "timeutil.h"
#include <ctime>
#include <string>
#include "luaapis.h" // IWYU pragma: keep

static int g_print(lua_State*);
static int g_require(lua_State*);
static const struct luaL_Reg luaglobals [] = {
    {"print", g_print},
    {"require", g_require},
    {NULL, NULL} /* end of array */
};

std::string unsafe_globals[] = {
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