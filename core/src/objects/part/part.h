#pragma once

#include "basepart.h"
#include "enum/part.h"
#include "objects/annotation.h"

class DEF_INST Part : public BasePart {
    AUTOGEN_PREAMBLE

    PartType _lastShape;
    Vector3 _lastSize;
protected:
    void updateCollider(rp::PhysicsCommon* common) override;

public:
    Part();
    Part(PartConstructParams params);

    DEF_PROP_(on_update=onUpdated) PartType shape = PartType::Block;

    Vector3 GetEffectiveSize() override;

    static inline std::shared_ptr<Part> New() { return std::make_shared<Part>(); };
    static inline std::shared_ptr<Part> New(PartConstructParams params) { return std::make_shared<Part>(params); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Part>(); };
};