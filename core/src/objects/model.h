#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"
#include <memory>

// Group object for Parts

class DEF_INST_(explorer_icon="model") Model : public Instance {
    AUTOGEN_PREAMBLE

public:
    Model();
    ~Model();

    static inline std::shared_ptr<Model> New() { return std::make_shared<Model>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Model>(); };
};