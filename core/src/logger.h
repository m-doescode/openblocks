#pragma once

#include <string>

namespace Logger {
    enum class LogLevel {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL_ERROR,
    };

    void init();
    void finish();

    void log(std::string message, LogLevel logLevel);
    inline void info(std::string message) { log(message, LogLevel::INFO); }
    inline void debug(std::string message) { log(message, LogLevel::DEBUG); }
    inline void warning(std::string message) { log(message, LogLevel::WARNING); }
    inline void error(std::string message) { log(message, LogLevel::ERROR); }
    inline void fatalError(std::string message) { log(message, LogLevel::FATAL_ERROR); }
};