#include "workspace.h"

static InstanceType TYPE_ {
    .super = Instance::TYPE,
    .className = "Workspace",
    .constructor = &Workspace::Create,
};

InstanceType* Workspace::TYPE = &TYPE_;

InstanceType* Workspace::GetClass() {
    return &TYPE_;
}

Workspace::Workspace(): Instance(&TYPE_) {
}
