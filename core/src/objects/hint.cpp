#include "hint.h"

INSTANCE_IMPL(Hint)

InstanceType Hint::__buildType() {
    return make_instance_type<Hint, Message>("Hint");
}

Hint::~Hint() = default;