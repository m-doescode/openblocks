#pragma once

#include <string>

#define DEF_WRAPPER_CLASS(CLASS_NAME, WRAPPED_TYPE) class CLASS_NAME {\
    WRAPPED_TYPE wrapped;\
public:\
    CLASS_NAME(WRAPPED_TYPE); \
    operator WRAPPED_TYPE(); \
};

namespace Data {
    class Void {
    public:
        Void();
    };

    DEF_WRAPPER_CLASS(Bool, bool)
    DEF_WRAPPER_CLASS(Int, int)
    DEF_WRAPPER_CLASS(Float, float)
    DEF_WRAPPER_CLASS(String, std::string)
};


#undef DEF_WRAPPER_CLASS