#pragma once

#include "result.h"

struct DUMMY_VALUE;

template <typename ...E>
class fallible : public result<DUMMY_VALUE, E...> {
};