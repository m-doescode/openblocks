#include "service.h"
#include "objects/datamodel.h"
#include "logger.h"
#include "panic.h"
#include <memory>

Service::Service(const InstanceType* type) : Instance(type) {}

// Fail if parented to non-datamodel, otherwise lock parent
void Service::OnParentUpdated(nullable std::shared_ptr<Instance> oldParent, nullable std::shared_ptr<Instance> newParent) {
    if (!newParent || newParent->GetClass() != &DataModel::TYPE) {
        Logger::fatalErrorf("Service %s was parented to object of type %s", GetClass()->className.c_str(), newParent ? newParent->GetClass()->className.c_str() : "NULL");
        panic();
    }

    // Prevent parent from being updated
    parentLocked = true;
}

void Service::InitService() {
}

void Service::OnRun() {
}