#pragma once

#include "datatypes/cframe.h"
#include <memory>
#include <vector>

class BasePart;
class Instance;
class Selection;

struct PartTransformState {
    std::shared_ptr<BasePart> part;
    Vector3 size;
    Vector3 velocity;
    Vector3 rotVelocity;
    CFrame cframe;
};

class PartAssembly {
    CFrame _assemblyOrigin;
    Vector3 _bounds;
    Vector3 _size;

    std::vector<std::shared_ptr<BasePart>> parts;
public:
    PartAssembly(std::vector<std::shared_ptr<BasePart>>, bool worldMode = false);

    static PartAssembly FromSelection(std::vector<std::shared_ptr<Instance>> selection);
    static PartAssembly FromSelection(std::shared_ptr<Selection> selection);

    inline CFrame assemblyOrigin() { return _assemblyOrigin; };
    inline Vector3 bounds() { return _bounds; };
    inline Vector3 size() { return _size; };
    inline bool multipleSelected() { return parts.size() > 1; }

    // Gets the current transform state of all the parts in the assembly
    std::vector<PartTransformState> GetCurrentTransforms();

    // Transforms the assembly such that newOrigin is now this assembly's new assemblyOrigin
    void SetOrigin(CFrame newOrigin);

    // Rotates and translates the assembly by the transformation
    void TransformBy(CFrame transform);

    // Scales the assembly to the desired size
    // If multiple parts are selected, finds the greatest scale factor of each component pair, and
    // scales it up by that amount
    void Scale(Vector3 newSize, bool scaleUp = true);

    // Update temporary collisions of rigidbodies. Useful for ignoring
    // items for raycasts
    void SetCollisionsEnabled(bool enabled);
};