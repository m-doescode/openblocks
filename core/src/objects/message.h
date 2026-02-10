#pragma once

#include "objectmodel/macro.h"
#include "objects/base/instance.h"
#include <memory>

// Dims the player's screen and displays some centered text
class Message : public Instance {
    INSTANCE_HEADER

public:
    ~Message();

    std::string text;

    static inline std::shared_ptr<Message> New() { return std::make_shared<Message>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Message>(); };
};