#include "error.h"
#include "logger.h"

Error::Error(std::string message) : message(message) {
}

std::string Error::getMessage() {
    return this->message;
}

void Error::logMessage(Logger::LogLevel logLevel) {
    Logger::log(this->message, logLevel);
}

void Error::logMessageFatal() {
    Logger::fatalError(this->message);
}