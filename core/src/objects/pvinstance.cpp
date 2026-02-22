#include "pvinstance.h"
#include "objectmodel/type.h"

INSTANCE_IMPL(PVInstance)

InstanceType PVInstance::__buildType() {
    return make_instance_type<PVInstance>("PVInstance", INSTANCE_NOTCREATABLE);
}

PVInstance::PVInstance() {}
PVInstance::~PVInstance() = default;