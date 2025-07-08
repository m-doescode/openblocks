#include "players.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"
#include <memory>

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