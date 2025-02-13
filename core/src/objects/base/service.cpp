#include "service.h"
#include <memory>

Service::Service(const InstanceType* type, std::weak_ptr<DataModel> root) : Instance(type), dataModel(root) {}

void Service::InitService() {
    SetParent(dataModel.lock());
    parentLocked = true;
}