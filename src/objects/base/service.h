#pragma once

// Services are top-level singletons and belong to a specific DataModel
// They serve one specific task and can be accessed using game:GetService
#include "objects/datamodel.h"
#include <memory>
class Service {
protected:
    std::weak_ptr<DataModel> dataModel;

    Service(std::weak_ptr<DataModel> root);
};