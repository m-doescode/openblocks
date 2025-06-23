#pragma once

#include "objects/annotation.h"
#include "objects/base/service.h"
#include "lua.h" // IWYU pragma: keep
#include <memory>
#include <vector>

struct SleepingThread {
    lua_State* thread;
    uint64_t timeYieldedWhen;
    uint64_t targetTimeMicros;
    bool active = true;
};

class Script;

class DEF_INST_SERVICE ScriptContext : public Service {
    AUTOGEN_PREAMBLE

    std::vector<SleepingThread> sleepingThreads;
    std::map<int, std::weak_ptr<Script>> scriptSources;
    int lastScriptSourceId = 0;
protected:
    void InitService() override;
    bool initialized = false;

public:
    ScriptContext();
    ~ScriptContext();

    lua_State* state;
    void PushThreadSleep(lua_State* thread, float delay);
    void RunSleepingThreads();
    std::string RegisterScriptSource(std::shared_ptr<Script>);
    std::weak_ptr<Script> GetScriptFromSource(std::string source);

    static inline std::shared_ptr<Instance> Create() { return std::make_shared<ScriptContext>(); };
};