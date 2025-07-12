#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"
#include <memory>

// Dims the player's screen and displays some centered text
class DEF_INST_(explorer_icon="message") Message : public Instance {
    AUTOGEN_PREAMBLE

protected:
    Message(const InstanceType* type);
public:
    Message();
    ~Message();

    DEF_PROP std::string text;

    static inline std::shared_ptr<Message> New() { return std::make_shared<Message>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Message>(); };
};