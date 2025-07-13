#pragma once

#include "datatypes/cframe.h"
#include "objects/part/part.h"
#include <array>
#include <memory>

class HandleFace {
    HandleFace(int index, glm::vec3 normal) : index(index), normal(normal){}

    public:
    int index;
    glm::vec3 normal;

    static HandleFace XPos;
    static HandleFace XNeg;
    static HandleFace YPos;
    static HandleFace YNeg;
    static HandleFace ZPos;
    static HandleFace ZNeg;
    static std::array<HandleFace, 6> Faces;
};

enum HandlesType {
    MoveHandles,
    ScaleHandles,
    RotateHandles,
};

struct Handles {
    bool nixAxes = false; // XYZ -> ZXY, used with rotation
    bool active;
    HandlesType handlesType;

    // World-space handles vs local-space handles
    bool worldMode = false;
};

std::shared_ptr<BasePart> getHandleAdornee();
CFrame getHandleCFrame(HandleFace face);
CFrame partCFrameFromHandlePos(HandleFace face, Vector3 newPos);
Vector3 handleSize(HandleFace face);
std::optional<HandleFace> raycastHandle(rp3d::Ray ray);

// Gets the cframe of the handle local to the center of the selected objects
CFrame getLocalHandleCFrame(HandleFace face);
