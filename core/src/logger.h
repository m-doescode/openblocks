#pragma once

#include <format>
#include <functional>
#include <string>

namespace Logger {
    enum class LogLevel {
        INFO,
        DEBUG,
        TRACE,
        WARNING,
        ERROR,
        FATAL_ERROR,
    };

    typedef std::function<void(LogLevel logLevel, std::string message)> LogListener;
    typedef std::function<void(std::string message, std::string source, int line, void* userData)> TraceLogListener;

    extern std::string currentLogDir;

    void init();
    void finish();
    void addLogListener(LogListener);
    void addLogListener(TraceLogListener);

    void log(std::string message, LogLevel logLevel);
    inline void info(std::string message) { log(message, LogLevel::INFO); }
    inline void debug(std::string message) { log(message, LogLevel::DEBUG); }
    inline void warning(std::string message) { log(message, LogLevel::WARNING); }
    inline void error(std::string message) { log(message, LogLevel::ERROR); }
    inline void fatalError(std::string message) { log(message, LogLevel::FATAL_ERROR); }

    inline void traceStart() { log("Stack start", LogLevel::TRACE); }
    inline void traceEnd() { log("Stack end", LogLevel::TRACE); }
    void trace(std::string source, int line, void* userData = nullptr);

    template <typename ...Args>
    void logf(std::string format, LogLevel logLevel, Args&&... args) {
        char message[200];
        sprintf(message, format.c_str(), args...);
        log(message, logLevel);
    }
    
    template <typename ...Args> inline void infof(std::string format, Args&&... args) { logf(format, LogLevel::INFO, args...); }
    template <typename ...Args> inline void debugf(std::string format, Args&&... args) { logf(format, LogLevel::DEBUG, args...); }
    template <typename ...Args> inline void warningf(std::string format, Args&&... args) { logf(format, LogLevel::WARNING, args...); }
    template <typename ...Args> inline void errorf(std::string format, Args&&... args) { logf(format, LogLevel::ERROR, args...); }
    template <typename ...Args> inline void fatalErrorf(std::string format, Args&&... args) { logf(format, LogLevel::FATAL_ERROR, args...);}
};