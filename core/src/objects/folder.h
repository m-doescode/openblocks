#pragma once

#include "objectmodel/macro.h"
#include "objects/base/instance.h"
#include <memory>

// The simplest instance
// Has no functionality of its own, used purely for organizational/grouping purposes

class Folder : public Instance {
    INSTANCE_HEADER

public:
    ~Folder();

    static inline std::shared_ptr<Folder> New() { return new_instance<Folder>(); };
    static inline std::shared_ptr<Instance> Create() { return new_instance<Folder>(); };
};