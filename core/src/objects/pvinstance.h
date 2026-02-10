#pragma once

#include "objectmodel/macro.h"
#include "objects/annotation.h"
#include "objects/base/instance.h"

class DEF_INST_ABSTRACT PVInstance : public Instance {
    INSTANCE_HEADER

protected:
    PVInstance();
public:
    ~PVInstance();
};