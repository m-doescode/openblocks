#pragma once

#include "objectmodel/macro.h"
#include "objects/base/service.h"
#include "luaapis.h" // IWYU pragma: keep
#include <memory>
#include <vector>

struct SleepingThread {
    lua_State* thread; // Dangerous, should only really be using the below
    int threadRef; // Ref used to keep the thread alive
    uint64_t timeYieldedWhen;
    uint64_t targetTimeMicros;
    bool active = true;
};

class Script;

class ScriptContext : public Service {
    INSTANCE_HEADER

    std::vector<SleepingThread> sleepingThreads;
    int lastScriptSourceId = 0;
protected:
    bool initialized = false;

public:
    ~ScriptContext();

    void InitService() override;
    
    lua_State* state;
    void PushThreadSleep(lua_State* thread, float delay);
    void RunSleepingThreads();
    // TEMPORARY. USED ONLY FOR TESTING
    void DebugClearSleepingThreads();

    // Generates an environment with a metatable and pushes it both the env table and metatable in order onto the stack
    void NewEnvironment(lua_State* state);

    static inline std::shared_ptr<Instance> Create() { return new_instance<ScriptContext>(); };
};