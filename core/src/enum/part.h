#pragma once

#include "datatypes/enum.h"
#include "enum/annotation.h"
#include "datatypes/enummeta.h"

enum class DEF_ENUM PartType {
    Ball = 0,
    Block = 1,
    Cylinder = 2,
};

namespace EnumType {
    extern const Enum PartType;
};

DEF_ENUM_META(PartType)