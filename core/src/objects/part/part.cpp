#include "part.h"
#include "enum/part.h"
#include "objectmodel/property.h"
#include <glm/common.hpp>

InstanceType Part::__buildType() {
    return make_instance_type<Part, BasePart>("Part",
        def_property("Shape", &Part::shape, 0, &Part::onUpdated)
    );
}

Part::Part() {
}

Part::Part(PartConstructParams params): BasePart(params) {
}

Vector3 Part::GetEffectiveSize() {
    float diameter;
    switch (shape) {
    case PartType::Ball:
        return (Vector3)glm::vec3(glm::min(size.X(), size.Y(), size.Z()));
    case PartType::Cylinder:
        diameter = glm::min(size.Y(), size.Z());
        return Vector3(size.X(), diameter, diameter);
    default:
        return size;
    }
}