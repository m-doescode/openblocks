#include "error.h"
#include "logger.h"

Error::Error(std::string message) : message(message) {
}

std::string Error::getMessage() {
    return this->message;
}

void Error::logMessage(Logger::LogLevel logLevel) {
    Logger::logf("%s", logLevel, this->message);
}

void Error::logMessageFatal() {
    Logger::fatalErrorf("%s", this->message);
}