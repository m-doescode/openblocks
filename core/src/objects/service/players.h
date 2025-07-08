#pragma once

#include "objects/annotation.h"
#include "objects/base/service.h"

class DEF_INST_SERVICE Players : public Service {
    AUTOGEN_PREAMBLE
protected:
    void InitService() override;
    bool initialized = false;

public:
    Players();
    ~Players();

    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Players>(); };

    DEF_PROP_CATEGORY(DATA)
    DEF_PROP_(readonly,no_save)
    int numPlayers = 0;
    DEF_PROP int maxPlayers = 6;
};