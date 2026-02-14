#include "clickdetector.h"
#include "objectmodel/signal.h"
#include "objectmodel/type.h"

InstanceType ClickDetector::__buildType() {
    return make_instance_type<ClickDetector>("ClickDetector",
        set_explorer_icon("mouse"),
        def_signal("MouseClick", &ClickDetector::MouseClick),
        def_signal("MouseHoverEnter", &ClickDetector::MouseHoverEnter),
        def_signal("MouseHoverLeave", &ClickDetector::MouseHoverLeave),
        def_signal("RightMouseClick", &ClickDetector::RightMouseClick)
    );
}