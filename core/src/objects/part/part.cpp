#include "part.h"
#include "enum/part.h"
#include <glm/common.hpp>

Part::Part(): BasePart(&TYPE) {
}

Part::Part(PartConstructParams params): BasePart(&TYPE, params) {
}

Vector3 Part::GetEffectiveSize() {
    return shape == PartType::Ball ? (Vector3)glm::vec3(glm::min(size.X(), size.Y(), size.Z())) : size;
}