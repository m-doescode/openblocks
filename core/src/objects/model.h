#pragma once

#include "objectmodel/macro.h"
#include "objects/base/instance.h"
#include "objects/pvinstance.h"
#include <memory>

// Group object for Parts

class Model : public PVInstance {
    INSTANCE_HEADER

public:
    ~Model();

    static inline std::shared_ptr<Model> New() { return std::make_shared<Model>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Model>(); };
};