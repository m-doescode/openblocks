#include "jointsservice.h"

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
