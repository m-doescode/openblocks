#include "workspace.h"

const InstanceType Workspace::TYPE = {
    .super = &Instance::TYPE,
    .className = "Workspace",
    .constructor = &Workspace::Create,
};

const InstanceType* Workspace::GetClass() {
    return &TYPE;
}

Workspace::Workspace(): Instance(&TYPE) {
}
