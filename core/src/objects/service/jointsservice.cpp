#include "jointsservice.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"
#include <memory>

JointsService::JointsService(): Service(&TYPE) {
}

JointsService::~JointsService() = default;

void JointsService::InitService() {
    if (initialized) return;
    initialized = true;

    // Clear children before any new joints are added
    for (std::shared_ptr<Instance> inst : GetChildren()) {
        inst->Destroy();
    }
}

nullable std::shared_ptr<Workspace> JointsService::jointWorkspace() {
    if (!dataModel()) return nullptr;

    return dataModel()->FindService<Workspace>();
}