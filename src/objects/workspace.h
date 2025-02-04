#pragma once

#include "base.h"
#include "objects/base/service.h"
#include <memory>

class Workspace : public Instance, Service {
//private:
public:
    const static InstanceType TYPE;

    Workspace(std::weak_ptr<DataModel> dataModel);

    // static inline std::shared_ptr<Workspace> New() { return std::make_shared<Workspace>(); };
    // static inline InstanceRef Create() { return std::make_shared<Workspace>(); };
    virtual const InstanceType* GetClass() override;
};