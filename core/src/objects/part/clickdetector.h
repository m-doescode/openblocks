#pragma once

#include "objectmodel/macro.h"
#include "objects/base/instance.h"

class ClickDetector : public Instance {
    INSTANCE_HEADER
public:
    SignalSource MouseClick;
    SignalSource MouseHoverEnter;
    SignalSource MouseHoverLeave;
    SignalSource RightMouseClick;

    static inline std::shared_ptr<ClickDetector> New() { return new_instance<ClickDetector>(); };
    static inline std::shared_ptr<Instance> Create() { return new_instance<ClickDetector>(); };
};