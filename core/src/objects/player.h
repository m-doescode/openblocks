#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"
#include "objects/model.h"
#include <memory>

class DEF_INST_(explorer_icon="player") Player : public Instance {
    AUTOGEN_PREAMBLE

public:
    Player();
    ~Player();

    DEF_PROP_CATEGORY(DATA)
    DEF_PROP_(readonly) int userId = 0;
    DEF_PROP std::weak_ptr<Model> character;
    // DEF_PROP_CATEGORY(TEAM) //placeholder, we cant add TeamColor or Teams yet since no BrickColor
    // DEF_PROP bool neutral = true;

    static inline std::shared_ptr<Player> New() { return std::make_shared<Player>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Player>(); };
};