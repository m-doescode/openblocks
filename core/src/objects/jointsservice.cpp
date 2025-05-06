#include "jointsservice.h"
#include "workspace.h"
#include "datamodel.h"
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

std::optional<std::shared_ptr<Workspace>> JointsService::jointWorkspace() {
    if (!dataModel()) return std::nullopt;

    return dataModel().value()->FindService<Workspace>();
}