#include "workspace.h"

const InstanceType Workspace::TYPE = {
    .super = &Instance::TYPE,
    .className = "Workspace",
    // .constructor = &Workspace::Create,
    .explorerIcon = "workspace",
};

const InstanceType* Workspace::GetClass() {
    return &TYPE;
}

Workspace::Workspace(std::weak_ptr<DataModel> dataModel): Instance(&TYPE), Service(dataModel) {
}
