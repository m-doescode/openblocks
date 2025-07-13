#pragma once

#include "basepart.h"
#include "objects/annotation.h"

class DEF_INST WedgePart : public BasePart {
    AUTOGEN_PREAMBLE

protected:
    void updateCollider(rp::PhysicsCommon* common) override;
    static void createWedgeShape(rp::PhysicsCommon* common);

    friend Workspace;
public:
    WedgePart();
    WedgePart(PartConstructParams params);

    static inline std::shared_ptr<WedgePart> New() { return std::make_shared<WedgePart>(); };
    static inline std::shared_ptr<WedgePart> New(PartConstructParams params) { return std::make_shared<WedgePart>(params); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<WedgePart>(); };
};