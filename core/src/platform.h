#pragma once

#include <string>

// Collection of platform-dependent APIs

// Gets the local data directory under the user's home directory
// Windows: %localappdata%/openblocks
// Linux: ~/.local/share/openblocks 
std::string getProgramDataDir();

// Gets the local logs directory under the program's data directory
// Windows: %localappdata%/openblocks/logs
// Linux: ~/.local/share/openblocks/logs
std::string getProgramLogsDir();

// Creates the local data directory
void initProgramDataDir();
// Creates the local logs directory
void initProgramLogsDir();

// Displays an error message box on Windows, or prints to eprintf
void printErrorMessage(std::string message);