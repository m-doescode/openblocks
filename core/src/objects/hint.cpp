#include "hint.h"

InstanceType Hint::__buildType() {
    return make_instance_type<Hint, Message>("Hint");
}

Hint::~Hint() = default;