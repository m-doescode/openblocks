#pragma once

#include "logger.h"
#include <string>

// Base class for all errors
class Error {
    std::string _errorType;
    std::string _message;

protected:
    inline Error(std::string errorType, std::string message) : _errorType(errorType), _message(message) {}

public:
    inline std::string errorType() { return this->_errorType; }
    inline std::string message() { return this->_message; }
    inline void logMessage(Logger::LogLevel logLevel = Logger::LogLevel::ERROR) { Logger::log(this->_message, logLevel); }
    void logMessageFatal() { Logger::fatalError(this->_message); }
};