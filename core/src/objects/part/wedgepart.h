#pragma once

#include "basepart.h"
#include "objectmodel/macro.h"

class DEF_INST WedgePart : public BasePart {
    INSTANCE_HEADER

public:
    WedgePart();
    WedgePart(PartConstructParams params);

    static inline std::shared_ptr<WedgePart> New() { return new_instance<WedgePart>(); };
    static inline std::shared_ptr<WedgePart> New(PartConstructParams params) { return new_instance<WedgePart>(params); };
    static inline std::shared_ptr<Instance> Create() { return new_instance<WedgePart>(); };
};