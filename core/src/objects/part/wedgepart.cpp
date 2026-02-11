#include "wedgepart.h"

InstanceType WedgePart::__buildType() {
    return make_instance_type<WedgePart, BasePart>("WedgePart");
}

WedgePart::WedgePart(): BasePart() {}

WedgePart::WedgePart(PartConstructParams params): BasePart(params) {}