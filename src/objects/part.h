#pragma once

#include "base.h"
#include <memory>

class Part : public Instance {
public:
    static InstanceType* TYPE;

    Part();

    static inline InstanceRef CreateGeneric() { return std::make_shared<Part>(); };
    virtual InstanceType* GetClass() override;
};