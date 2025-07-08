#include "player.h"
#include "objects/service/players.h"
#include "objects/datamodel.h"

Player::Player(): Instance(&TYPE) {
    dataModel().value()->GetService<Players>()->numPlayers += 1;
}
Player::~Player() {
    // Subtract from player count on remove
    dataModel().value()->GetService<Players>()->numPlayers -= 1;
}