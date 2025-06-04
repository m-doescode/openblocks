#pragma once

#include "datatypes/enum.h"
#include "enum/annotation.h"

enum DEF_ENUM NormalId {
    Right = 0,
    Top = 1,
    Back = 2,
    Left = 3,
    Bottom = 4,
    Front = 5
};

enum DEF_ENUM SurfaceType {
    SurfaceSmooth = 0,
    SurfaceGlue = 1,
    SurfaceWeld = 2,
    SurfaceStuds = 3,
    SurfaceInlets = 4,
    SurfaceUniversal = 5,
    SurfaceHinge = 6,
    SurfaceMotor = 7,
};

namespace EnumType {
    extern const Enum NormalId;
    extern const Enum SurfaceType;
};

class Vector3;
NormalId faceFromNormal(Vector3);
Vector3 normalFromFace(NormalId);