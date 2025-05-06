#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"
#include <memory>

class DEF_INST_(explorer_icon="script") Script : public Instance {
    AUTOGEN_PREAMBLE

    lua_State* thread;
public:
    Script();
    ~Script();

    [[ def_prop(name="Source", hidden) ]]
    std::string source;
    void Run();
    void Stop();

    static inline std::shared_ptr<Script> New() { return std::make_shared<Script>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Script>(); };
};