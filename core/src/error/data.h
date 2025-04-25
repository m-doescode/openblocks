#pragma once

#include "error.h"

class LuaCastError : public Error {
    public:
    inline LuaCastError(std::string sourceType, std::string targetType) : Error("InstanceCastError", "Attempt to cast " + sourceType + " to " + targetType) {}
};