#pragma once

#include <variant>
#include "datatypes/primitives.h"

typedef std::variant<StringData, IntData, FloatData> DataValue;

const uint DAT_STRING = 0;
const uint DAT_INT = 1;
const uint DAT_FLOAT = 2;