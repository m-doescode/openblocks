#pragma once

#include "base.h"
#include <memory>

class Workspace : public Instance {
//private:
public:
    static InstanceType* TYPE;

    Workspace();

    static inline std::shared_ptr<Workspace> New() { return std::make_shared<Workspace>(); };
    static inline InstanceRef Create() { return std::make_shared<Workspace>(); };
    virtual InstanceType* GetClass() override;
};