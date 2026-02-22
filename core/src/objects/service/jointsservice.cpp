#include "jointsservice.h"
#include "objectmodel/type.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"
#include <memory>

INSTANCE_IMPL(JointsService)

InstanceType JointsService::__buildType() {
    return make_instance_type<JointsService>("JointsService", INSTANCE_NOTCREATABLE | INSTANCE_SERVICE);
}

JointsService::~JointsService() = default;

void JointsService::InitService() {
    if (initialized) return;
    initialized = true;

    // Clear children before any new joints are added
    for (std::shared_ptr<Instance> inst : GetChildren()) {
        inst->Destroy();
    }
}

nullable std::shared_ptr<Workspace> JointsService::jointWorkspace() {
    if (!dataModel()) return nullptr;

    return dataModel()->FindService<Workspace>();
}