#pragma once

#include "basepart.h"
#include "objects/annotation.h"

class DEF_INST Part : public BasePart {
    AUTOGEN_PREAMBLE

public:
    Part();
    Part(PartConstructParams params);

    static inline std::shared_ptr<Part> New() { return std::make_shared<Part>(); };
    static inline std::shared_ptr<Part> New(PartConstructParams params) { return std::make_shared<Part>(params); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Part>(); };
};