#include "jointsservice.h"
#include "workspace.h"
#include <memory>

const InstanceType JointsService::TYPE = {
    .super = &Instance::TYPE,
    .className = "JointsService",
    .constructor = &JointsService::Create,
    .flags = INSTANCE_NOTCREATABLE | INSTANCE_SERVICE | INSTANCE_HIDDEN,
};

const InstanceType* JointsService::GetClass() {
    return &TYPE;
}


JointsService::JointsService(): Service(&TYPE) {
}

JointsService::~JointsService() = default;

void JointsService::InitService() {
    if (initialized) return;
    initialized = true;

    // Clear children before any new joints are added
    for (std::shared_ptr<Instance> inst : dataModel().value()->GetService<JointsService>()->GetChildren()) {
        inst->Destroy();
    }
}

std::optional<std::shared_ptr<Workspace>> JointsService::jointWorkspace() {
    if (!dataModel()) return std::nullopt;

    return dataModel().value()->FindService<Workspace>();
}