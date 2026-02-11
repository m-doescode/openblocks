#pragma once

#include "objectmodel/macro.h"
#include "objects/base/instance.h"
#include <memory>

class Script : public Instance {
    INSTANCE_HEADER

    lua_State* thread;
public:
    Script();
    ~Script();

    std::string source;
    void Run();
    void Stop();

    static inline std::shared_ptr<Script> New() { return new_instance<Script>(); };
    static inline std::shared_ptr<Instance> Create() { return new_instance<Script>(); };
};