#pragma once

#include "objects/base/service.h"
#include "lua.h"

class ScriptContext : public Service {
private:
protected:
    void InitService() override;
    bool initialized = false;

public:
    const static InstanceType TYPE;

    ScriptContext();
    ~ScriptContext();

    lua_State* state;

    static inline std::shared_ptr<Instance> Create() { return std::make_shared<ScriptContext>(); };
    virtual const InstanceType* GetClass() override;
};