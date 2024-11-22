#include "primitives.h"

StringData::StringData(std::string str) : wrapped(str) {}
StringData::operator std::string() { return wrapped; }

IntData::IntData(int in) : wrapped(in) {}
IntData::operator int() { return wrapped; }

FloatData::FloatData(float in) : wrapped(in) {}
FloatData::operator float() { return wrapped; }

