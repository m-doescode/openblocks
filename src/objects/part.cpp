#include "part.h"

static InstanceType TYPE_ {
    .super = Instance::TYPE,
    .className = "Part",
    .constructor = &Part::CreateGeneric,
};

InstanceType* Part::TYPE = &TYPE_;

InstanceType* Part::GetClass() {
    return &TYPE_;
}

Part::Part(): Instance(&TYPE_) {
}
