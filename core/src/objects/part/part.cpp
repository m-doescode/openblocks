#include "part.h"
#include "enum/part.h"
#include <glm/common.hpp>

Part::Part(): BasePart(&TYPE) {
}

Part::Part(PartConstructParams params): BasePart(&TYPE, params) {
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