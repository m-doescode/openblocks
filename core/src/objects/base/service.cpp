#include "service.h"
#include "objects/datamodel.h"
#include "logger.h"
#include "panic.h"
#include <memory>

// Fail if parented to non-datamodel, otherwise lock parent
void Service::OnParentUpdated(nullable std::shared_ptr<Instance> oldParent, nullable std::shared_ptr<Instance> newParent) {
    if (!newParent || newParent->GetType() != DataModel::Type()) {
        Logger::fatalErrorf("Service %s was parented to object of type %s", GetType().className.c_str(), newParent ? newParent->GetType().className.c_str() : "NULL");
        panic();
    }

    // Prevent parent from being updated
    parentLocked = true;
}

void Service::InitService() {
}

void Service::OnRun() {
}