#pragma once

#include "error.h"

class LuaCastError : public Error {
    public:
    inline LuaCastError(std::string sourceType, std::string targetType) : Error("LuaCastError", "Attempt to cast " + sourceType + " to " + targetType) {}
};

class DataParseError : public Error {
    public:
    inline DataParseError(std::string parsedString, std::string targetType) : Error("DataParseError", "Failed to parse '" + parsedString + "' into value of type " + targetType) {}
};