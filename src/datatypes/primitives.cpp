#include "primitives.h"

#define IMPL_WRAPPER_CLASS(CLASS_NAME, WRAPPED_TYPE) Data::CLASS_NAME::CLASS_NAME(WRAPPED_TYPE in) : wrapped(in) {}\
Data::CLASS_NAME::operator WRAPPED_TYPE() { return wrapped; }

Data::Void::Void() {};

IMPL_WRAPPER_CLASS(Bool, bool)
IMPL_WRAPPER_CLASS(Int, int)
IMPL_WRAPPER_CLASS(Float, float)
IMPL_WRAPPER_CLASS(String, std::string)