#pragma once

#include "objects/annotation.h"
#include "objects/base/service.h"
#include "luaapis.h" // IWYU pragma: keep
#include <memory>
#include <vector>

struct SleepingThread {
    lua_State* thread;
    uint64_t timeYieldedWhen;
    uint64_t targetTimeMicros;
    bool active = true;
};

class Script;

class DEF_INST_SERVICE_(hidden) ScriptContext : public Service {
    AUTOGEN_PREAMBLE

    std::vector<SleepingThread> sleepingThreads;
    int lastScriptSourceId = 0;
protected:
    bool initialized = false;

public:
    ScriptContext();
    ~ScriptContext();

    void InitService() override;
    
    lua_State* state;
    void PushThreadSleep(lua_State* thread, float delay);
    void RunSleepingThreads();

    // Generates an environment with a metatable and pushes it both the env table and metatable in order onto the stack
    void NewEnvironment(lua_State* state);

    static inline std::shared_ptr<Instance> Create() { return std::make_shared<ScriptContext>(); };
};