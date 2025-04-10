#include "workspace.h"
#include "objects/base/instance.h"

const InstanceType Workspace::TYPE = {
    .super = &Instance::TYPE,
    .className = "Workspace",
    .constructor = &Workspace::Create,
    .explorerIcon = "workspace",
    .flags = INSTANCE_NOTCREATABLE | INSTANCE_SERVICE,
};

const InstanceType* Workspace::GetClass() {
    return &TYPE;
}

Workspace::Workspace(): Service(&TYPE) {
}
