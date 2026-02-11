#pragma once

#include "datatypes/enum.h"
#include "datatypes/enummeta.h"
#include "enum/annotation.h"

enum DEF_ENUM NormalId {
    Right = 0,
    Top = 1,
    Back = 2,
    Left = 3,
    Bottom = 4,
    Front = 5
};

enum class DEF_ENUM SurfaceType {
    Smooth = 0,
    Glue = 1,
    Weld = 2,
    Studs = 3,
    Inlet = 4,
    Universal = 5,
    Hinge = 6,
    Motor = 7,
};

namespace EnumType {
    extern const Enum NormalId;
    extern const Enum SurfaceType;
};

DEF_ENUM_META(NormalId)
DEF_ENUM_META(SurfaceType)

class Vector3;
NormalId faceFromNormal(Vector3);
Vector3 normalFromFace(NormalId);