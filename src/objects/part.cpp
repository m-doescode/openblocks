#include "part.h"
#include "base/instance.h"

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

Part::Part(PartConstructParams params): Instance(&TYPE_), position(params.position), rotation(params.rotation),
                                        scale(params.scale), material(params.material), anchored(params.anchored) {
}