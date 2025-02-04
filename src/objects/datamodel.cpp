#include "datamodel.h"
#include "workspace.h"
#include <memory>

const InstanceType DataModel::TYPE = {
    .super = &Instance::TYPE,
    .className = "DataModel",
    .constructor = nullptr,
};

const InstanceType* DataModel::GetClass() {
    return &TYPE;
}

DataModel::DataModel()
    : Instance(&TYPE) {
}

void DataModel::Init() {
    this->workspace = std::make_shared<Workspace>(shared<DataModel>());
    this->AddChild(this->workspace);
}