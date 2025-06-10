#pragma once

#include "datatypes/cframe.h"
#include <memory>
#include <vector>

class Part;
class Instance;

const std::vector<std::shared_ptr<Instance>> getSelection();

class PartAssembly {
    CFrame _assemblyOrigin;
    Vector3 _bounds;
    Vector3 _size;

    std::vector<std::shared_ptr<Part>> parts;
public:
    PartAssembly(std::vector<std::shared_ptr<Part>>, bool worldMode = false);

    static PartAssembly FromSelection(std::vector<std::shared_ptr<Instance>> selection = getSelection());

    inline CFrame assemblyOrigin() { return _assemblyOrigin; };
    inline Vector3 bounds() { return _bounds; };
    inline Vector3 size() { return _size; };

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