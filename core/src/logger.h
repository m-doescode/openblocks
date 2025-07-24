#pragma once

#include <functional>
#include <memory>
#include <ostream>
#include <string>

class Script;

namespace Logger {
    enum class LogLevel {
        INFO,
        DEBUG,
        TRACE,
        WARNING,
        ERROR,
        FATAL_ERROR,
    };

    struct ScriptSource {
        std::shared_ptr<Script> script;
        int line;
    };

    typedef std::function<void(LogLevel logLevel, std::string message, ScriptSource source)> LogListener;

    extern std::string currentLogDir;

    void init();
    void initTest(std::stringstream* out); // Testing only!
    void finish();
    void addLogListener(LogListener);
    void resetLogListeners(); // Testing only!

    void log(std::string message, LogLevel logLevel, ScriptSource source = {});
    inline void info(std::string message) { log(message, LogLevel::INFO); }
    inline void debug(std::string message) { log(message, LogLevel::DEBUG); }
    inline void warning(std::string message) { log(message, LogLevel::WARNING); }
    inline void error(std::string message) { log(message, LogLevel::ERROR); }
    inline void fatalError(std::string message) { log(message, LogLevel::FATAL_ERROR); }
    inline void trace(std::string message) { log(message, LogLevel::TRACE); };

    template <typename ...Args>
    void scriptLogf(std::string format, LogLevel logLevel, ScriptSource source, Args&&... args) {
        char message[200];
        sprintf(message, format.c_str(), args...);
        log(message, logLevel, source);
    }

    template <typename ...Args>
    void logf(std::string format, LogLevel logLevel, Args&&... args) {
        scriptLogf(format, logLevel, {}, args...);
    }

    template <typename ...Args> inline void infof(std::string format, Args&&... args) { logf(format, LogLevel::INFO, args...); }
    template <typename ...Args> inline void debugf(std::string format, Args&&... args) { logf(format, LogLevel::DEBUG, args...); }
    template <typename ...Args> inline void warningf(std::string format, Args&&... args) { logf(format, LogLevel::WARNING, args...); }
    template <typename ...Args> inline void errorf(std::string format, Args&&... args) { logf(format, LogLevel::ERROR, args...); }
    template <typename ...Args> inline void fatalErrorf(std::string format, Args&&... args) { logf(format, LogLevel::FATAL_ERROR, args...);}
};