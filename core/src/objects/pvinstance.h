#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"

class DEF_INST_ABSTRACT PVInstance : public Instance {
    AUTOGEN_PREAMBLE

protected:
    PVInstance(const InstanceType*);
public:
    ~PVInstance();
};