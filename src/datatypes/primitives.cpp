#include "primitives.h"

#define IMPL_WRAPPER_CLASS(CLASS_NAME, WRAPPED_TYPE) CLASS_NAME::CLASS_NAME(WRAPPED_TYPE in) : wrapped(in) {}\
CLASS_NAME::operator WRAPPED_TYPE() { return wrapped; }

VoidData::VoidData() {};

IMPL_WRAPPER_CLASS(BoolData, bool)
IMPL_WRAPPER_CLASS(IntData, int)
IMPL_WRAPPER_CLASS(FloatData, float)
IMPL_WRAPPER_CLASS(StringData, std::string)