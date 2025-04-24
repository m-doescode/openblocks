#pragma once

#include "objects/base/instance.h"
#include <memory>

class Script : public Instance {
public:
    const static InstanceType TYPE;

    Script();
    ~Script();

    std::string source;
    void Run();
    void Stop();

    static inline std::shared_ptr<Script> New() { return std::make_shared<Script>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Script>(); };
    virtual const InstanceType* GetClass() override;
};