#include "message.h"
#include "objectmodel/type.h"

InstanceType Message::__buildType() {
    return make_instance_type<Message>("Message", set_explorer_icon("message"));
}

Message::~Message() = default;