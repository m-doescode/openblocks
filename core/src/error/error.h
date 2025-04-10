#pragma once

#include "logger.h"
#include <string>

// Base class for all errors
class Error {
    std::string message;

protected:
    Error(std::string message);

public:
    std::string getMessage();
    void logMessage(Logger::LogLevel logLevel = Logger::LogLevel::ERROR);
    void logMessageFatal();
};