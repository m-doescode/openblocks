#include "partassembly.h"
#include "common.h"
#include "datatypes/cframe.h"
#include "datatypes/variant.h"
#include "datatypes/vector.h"
#include "math_helper.h"
#include "objects/base/instance.h"
#include "objects/part/part.h"
#include "objects/service/selection.h"
#include <glm/common.hpp>
#include <memory>
#include <vector>

PartAssembly::PartAssembly(std::vector<std::shared_ptr<BasePart>> parts, bool worldMode) : parts(parts) {
    if (parts.size() == 0) return;
    if (parts.size() == 1 && !worldMode) {
        _assemblyOrigin = parts[0]->cframe;
        _size = parts[0]->size;
        _bounds = parts[0]->GetAABB();
        return;
    }

    glm::vec3 min = parts[0]->position(), max = parts[0]->position();

    for (auto part : parts) {
        Vector3 aabbSize = part->GetAABB();
        expandAABB(min, max, part->position() - aabbSize / 2.f);
        expandAABB(min, max, part->position() + aabbSize / 2.f);
    }

    glm::vec3 pos, size;
    getAABBCoords(pos, size, min, max);

    _assemblyOrigin = CFrame() + pos;
    _size = _bounds = size;
}

PartAssembly PartAssembly::FromSelection(std::vector<std::shared_ptr<Instance>> newSelection) {
    std::vector<std::shared_ptr<BasePart>> selection;

    for (std::shared_ptr<Instance> obj : newSelection) {
        if (!obj->IsA<PVInstance>()) continue;

        if (obj->IsA<BasePart>())
            selection.push_back(obj->CastTo<BasePart>().expect());

        // Add object descendants
        for (DescendantsIterator it = obj->GetDescendantsStart(); it != obj->GetDescendantsEnd(); it++) {
            if (!(*it)->IsA<BasePart>()) continue;

            selection.push_back((*it)->CastTo<BasePart>().expect());
        }
    }

    return PartAssembly(selection, editorToolHandles.worldMode);
}

PartAssembly PartAssembly::FromSelection(std::shared_ptr<Selection> selection) {
    return FromSelection(selection->Get());
}

void PartAssembly::SetCollisionsEnabled(bool enabled) {
    for (auto part : parts) {
        part->rigidBody->getCollider(0)->setIsWorldQueryCollider(enabled);
    }
}

void PartAssembly::SetOrigin(CFrame newOrigin) {
    for (auto part : parts) {
        part->cframe = newOrigin * (_assemblyOrigin.Inverse() * part->cframe);
        part->UpdateProperty("CFrame");
        // sendPropertyUpdatedSignal(part, "CFrame", Variant(part->cframe));
    }

    _assemblyOrigin = newOrigin;
}

void PartAssembly::TransformBy(CFrame transform) {
    for (auto part : parts) {
        part->cframe = transform * part->cframe;
        part->UpdateProperty("CFrame");
        part->UpdateProperty("Position");
        part->UpdateProperty("Rotation");
        sendPropertyUpdatedSignal(part, "CFrame", Variant(part->cframe));
        sendPropertyUpdatedSignal(part, "Position", Variant(part->cframe));
        sendPropertyUpdatedSignal(part, "Rotation", Variant(part->cframe));
    }

    _assemblyOrigin = transform * _assemblyOrigin;
}

void PartAssembly::Scale(Vector3 newSize, bool scaleUp) {
    if (parts.size() == 1 && (!parts[0]->IsA<Part>() || parts[0]->CastTo<Part>().expect()->shape != PartType::Ball)) {
        parts[0]->size = newSize;
        parts[0]->UpdateProperty("Size");
        sendPropertyUpdatedSignal(parts[0], "Size", Variant(parts[0]->size));
        _size = _bounds = newSize;
        return;
    }

    float sx = newSize.X() / _bounds.X(), sy = newSize.Y() / _bounds.Y(), sz = newSize.Z() / _bounds.Z();
    float factor = scaleUp ? glm::max(sx, sy, sz) : glm::min(sx, sy, sz);

    for (auto part : parts) {
        Vector3 localOff = _assemblyOrigin.Inverse() * part->cframe.Position();
        localOff = localOff * factor;
        part->cframe = part->cframe.Rotation() + _assemblyOrigin * localOff;
        part->UpdateProperty("CFrame");
        part->UpdateProperty("Position");
        part->UpdateProperty("Rotation");
        sendPropertyUpdatedSignal(part, "CFrame", Variant(part->cframe));
        sendPropertyUpdatedSignal(part, "Position", Variant(part->cframe));
        sendPropertyUpdatedSignal(part, "Rotation", Variant(part->cframe));
        part->size *= factor;
        part->UpdateProperty("Size");
        sendPropertyUpdatedSignal(part, "Size", Variant(part->size));
    }

    _size = _bounds = _bounds * factor;
}

std::vector<PartTransformState> PartAssembly::GetCurrentTransforms() {
    std::vector<PartTransformState> transforms;

    for (auto part : parts) {
        PartTransformState t;
        t.part = part;
        t.cframe = part->cframe;
        t.size = part->size;
        transforms.push_back(t);
    }

    return transforms;
}