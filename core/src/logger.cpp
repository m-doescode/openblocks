#include "logger.h"
#include "platform.h"

#include <fstream>
#include <chrono>
#include <format>
#include <string>
#include <vector>

static std::ofstream logStream;
static std::vector<Logger::LogListener> logListeners;
std::string Logger::currentLogDir = "NULL";

void Logger::init() {
    initProgramLogsDir();

    const auto now = std::chrono::system_clock::now();

    std::string fileName = std::format("log_{0:%Y%m%d}_{0:%H%M%S}.txt", now);

    logStream = std::ofstream(currentLogDir = (getProgramLogsDir() + "/" + fileName));
    Logger::debug("Logger initialized");
}

void Logger::finish() {
    Logger::debug("Closing logger...");
    logStream.close();
}

void Logger::log(std::string message, Logger::LogLevel logLevel, ScriptSource source) {
    std::string logLevelStr = logLevel == Logger::LogLevel::INFO ? "INFO" : 
        logLevel == Logger::LogLevel::DEBUG ? "DEBUG" :
        logLevel == Logger::LogLevel::TRACE ? "TRACE" :
        logLevel == Logger::LogLevel::WARNING ? "WARN" :
        logLevel == Logger::LogLevel::ERROR ? "ERROR" :
        logLevel == Logger::LogLevel::FATAL_ERROR ? "FATAL" : "?";

    const auto now = std::chrono::system_clock::now();

    std::string formattedLogLine = std::format("[{:%Y-%m-%d %X}] [{}] {}", now, logLevelStr, message);

    logStream << formattedLogLine << std::endl;
    printf("%s\n", formattedLogLine.c_str());

    for (Logger::LogListener listener : logListeners) {
        listener(logLevel, message, source);
    }

    if (logLevel == Logger::LogLevel::FATAL_ERROR) {
        displayErrorMessage(message);
    }
}

void Logger::addLogListener(Logger::LogListener listener) {
    logListeners.push_back(listener);
}