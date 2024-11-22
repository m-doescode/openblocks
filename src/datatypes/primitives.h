#pragma once

#include <string>

#define DEF_WRAPPER_CLASS(CLASS_NAME, WRAPPED_TYPE) class CLASS_NAME {\
    WRAPPED_TYPE wrapped;\
public:\
    CLASS_NAME(WRAPPED_TYPE); \
    operator WRAPPED_TYPE(); \
};

class VoidData {
public:
    VoidData();
};

DEF_WRAPPER_CLASS(BoolData, bool)
DEF_WRAPPER_CLASS(IntData, int)
DEF_WRAPPER_CLASS(FloatData, float)
DEF_WRAPPER_CLASS(StringData, std::string)

#undef DEF_WRAPPER_CLASS