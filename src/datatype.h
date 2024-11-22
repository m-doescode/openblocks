#pragma once

#include <variant>
#include "datatypes/primitives.h"

typedef std::variant<VoidData, BoolData, StringData, IntData, FloatData> DataValue;

enum class DataType {
    VOID = 0,
    BOOL = 1,
    STRING = 2,
    INT = 3,
    FLOAT = 4
};