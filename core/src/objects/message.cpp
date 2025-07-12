#include "message.h"

Message::Message(const InstanceType* type) : Instance(type) {}
Message::Message(): Instance(&TYPE) {}
Message::~Message() = default;