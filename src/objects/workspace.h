#pragma once

#include "base.h"
#include "objects/base/service.h"
#include <memory>

class Workspace : public Service {
//private:
public:
    const static InstanceType TYPE;

    Workspace(std::weak_ptr<DataModel> dataModel);

    // static inline std::shared_ptr<Workspace> New() { return std::make_shared<Workspace>(); };
    static inline std::shared_ptr<Service> Create(std::weak_ptr<DataModel> parent) { return std::make_shared<Workspace>(parent); };
    virtual const InstanceType* GetClass() override;
};