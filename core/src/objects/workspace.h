#pragma once

#include "base.h"
#include "objects/base/service.h"
#include <memory>

class Workspace : public Service {
//private:
public:
    const static InstanceType TYPE;

    Workspace();

    // static inline std::shared_ptr<Workspace> New() { return std::make_shared<Workspace>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Workspace>(); };
    virtual const InstanceType* GetClass() override;
};