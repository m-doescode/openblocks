#pragma once

#include "objects/annotation.h"
#include "objects/base/instance.h"
#include "objects/message.h"
#include <memory>

// Dims the player's screen and displays some centered text
class DEF_INST_(explorer_icon="message") Hint : public Message {
    AUTOGEN_PREAMBLE

public:
    Hint();
    ~Hint();

    static inline std::shared_ptr<Hint> New() { return std::make_shared<Hint>(); };
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Hint>(); };
};