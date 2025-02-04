#pragma once

// Services are top-level singletons and belong to a specific DataModel
// They serve one specific task and can be accessed using game:GetService
#include "objects/datamodel.h"
#include <memory>
class Service : public Instance {
protected:
    std::weak_ptr<DataModel> dataModel;

    Service(const InstanceType* type, std::weak_ptr<DataModel> root);
    virtual void InitService();

    friend class DataModel;
};