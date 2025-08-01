#pragma once

#include "datatypes/enum.h"
#include "enum/annotation.h"


enum class DEF_ENUM PartType {
    Ball = 0,
    Block = 1,
};

namespace EnumType {
    extern const Enum PartType;
};