#pragma once

#include "objects/annotation.h"
#include "objects/base/service.h"
#include "lua.h"

class INSTANCE_SERVICE() ScriptContext : public Service {
    AUTOGEN_PREAMBLE
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