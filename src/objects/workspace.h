#pragma once

#include "base.h"
#include <memory>

class Workspace : public Instance {
//private:
public:
    const static InstanceType TYPE;

    Workspace();

    static inline std::shared_ptr<Workspace> New() { return std::make_shared<Workspace>(); };
    static inline InstanceRef Create() { return std::make_shared<Workspace>(); };
    virtual const InstanceType* GetClass() override;
};