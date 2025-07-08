#pragma once

#include "objects/annotation.h"
#include "objects/base/service.h"
#include "objects/player.h"

class DEF_INST_SERVICE_(explorer_icon="players") Players : public Service {
    AUTOGEN_PREAMBLE
protected:
    void InitService() override;
    bool initialized = false;
public:
    Players();
    ~Players();

    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Players>(); };

    std::optional<std::shared_ptr<Player>> createLocalPlayer(int userId);
    void removeLocalPlayer();

    DEF_PROP_CATEGORY(DATA)
    DEF_PROP_(readonly,no_save) int numPlayers = 0;
    DEF_PROP int maxPlayers = 10;
    DEF_PROP_(readonly,no_save) std::weak_ptr<Player> localPlayer;
};