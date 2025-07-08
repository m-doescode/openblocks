#include "players.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"
#include "objects/player.h"
#include <memory>

int numPlayers;
int maxPlayers;

Players::Players(): Service(&TYPE) {
}

Players::~Players() = default;

void Players::InitService() {
    if (initialized) return;
    initialized = true;

    // Clear any players if they for some reason exist
    for (std::shared_ptr<Instance> inst : GetChildren()) {
        inst->Destroy();
    }
}

std::shared_ptr<Player> Players::createLocalPlayer(int userId) {
    std::shared_ptr<Player> newPlr = Player::New();
    newPlr->name = "Player"+std::to_string(userId);
    newPlr->userId = userId;
    dataModel().value()->GetService<Players>()->AddChild(newPlr);
    return newPlr;
}