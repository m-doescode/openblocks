#include "players.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"
#include "objects/player.h"
#include <memory>
#include <string>
#include <iostream>

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

std::optional<std::shared_ptr<Player>> Players::createLocalPlayer(int userId) {
    if (!dataModel()) return std::nullopt;
    std::shared_ptr<Player> newPlr = Player::New();
    newPlr->name = "Player"+std::to_string(userId);
    newPlr->userId = userId;
    this->localPlayer = newPlr;
    this->AddChild(newPlr);
    return newPlr;
}

void Players::removeLocalPlayer() {
    if (!this->localPlayer.lock()) return;
    this->localPlayer.lock()->Destroy();
    this->localPlayer = std::weak_ptr<Player>();
}