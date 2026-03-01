#include "camera.h"
#include "objectmodel/property.h"
#include "objectmodel/type.h"

INSTANCE_IMPL(Camera)

InstanceType Camera::__buildType() {
    return make_instance_type<Camera>("Camera",
        set_explorer_icon("camera"),

        def_property("CFrame", &Camera::cframe),
        def_property("CameraSubject", &Camera::cameraSubject),
        def_property("FieldOfView", &Camera::fieldOfView)
    );
}