#pragma once

#include <filesystem>
#include <string>

void loadCaches(std::string outDir);
void flushCaches(std::string outDir);

bool hasFileBeenUpdated(std::string path);
void markFileCached(std::string path);