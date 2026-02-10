#include "message.h"

InstanceType Message::__buildType() {
    return make_instance_type<Message>("Message");
}

Message::~Message() = default;