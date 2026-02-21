#include "message.h"
#include "objectmodel/type.h"

INSTANCE_IMPL(Message)

InstanceType Message::__buildType() {
    return make_instance_type<Message>("Message", set_explorer_icon("message"));
}

Message::~Message() = default;