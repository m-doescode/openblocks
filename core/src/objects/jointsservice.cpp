#include "jointsservice.h"
#include "workspace.h"

const InstanceType JointsService::TYPE = {
    .super = &Instance::TYPE,
    .className = "JointsService",
    .constructor = &JointsService::Create,
    .flags = INSTANCE_NOTCREATABLE | INSTANCE_SERVICE,
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
}

std::optional<std::shared_ptr<Workspace>> JointsService::jointWorkspace() {
    if (!dataModel()) return std::nullopt;

    return dataModel().value()->FindService<Workspace>();
}