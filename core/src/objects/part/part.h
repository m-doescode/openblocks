#pragma once

#include "basepart.h"
#include "enum/part.h"
#include "objectmodel/macro.h"

class Part : public BasePart {
    INSTANCE_HEADER
public:
    Part();
    Part(PartConstructParams params);

    PartType shape = PartType::Block;

    Vector3 GetEffectiveSize() override;

    static inline std::shared_ptr<Part> New() { return std::make_shared<Part>(); };
    static inline std::shared_ptr<Part> New(PartConstructParams params) { return std::make_shared<Part>(params); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Part>(); };
};