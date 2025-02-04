#pragma once

#include "base.h"
#include <memory>

class Workspace;

// The root instance to all objects in the hierarchy
class DataModel : public Instance {
//private:
public:
    const static InstanceType TYPE;

    std::shared_ptr<Workspace> workspace;

    DataModel();
    void Init();

    static inline std::shared_ptr<DataModel> New() { return std::make_shared<DataModel>(); };
    virtual const InstanceType* GetClass() override;
};