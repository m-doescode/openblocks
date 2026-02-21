#include "wedgepart.h"

INSTANCE_IMPL(WedgePart)

InstanceType WedgePart::__buildType() {
    return make_instance_type<WedgePart, BasePart>("WedgePart");
}

WedgePart::WedgePart(): BasePart() {}

WedgePart::WedgePart(PartConstructParams params): BasePart(params) {}