#pragma once

#include "objectmodel/macro.h"
#include "objects/base/instance.h"
#include "objects/message.h"
#include <memory>

// Dims the player's screen and displays some centered text
class Hint : public Message {
    INSTANCE_HEADER

public:
    ~Hint();

    static inline std::shared_ptr<Hint> New() { return new_instance<Hint>(); };
    static inline std::shared_ptr<Instance> Create() { return new_instance<Hint>(); };
};