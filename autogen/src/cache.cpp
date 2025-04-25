#include "cache.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

extern std::map<std::string, std::vector<std::string>> SUBCLASSES;
extern std::map<std::string, std::string> SUPERCLASS;

std::map<std::string, uint64_t> LAST_MODIFIED_TIMES;

void loadModTimes(std::string path);
void writeModTimes(std::string path);

void loadCaches(std::string outDir) {
    fs::path cacheDir = fs::path(outDir) / ".cache";
    if (!fs::exists(cacheDir)) return;

    fs::path modtimesFile = cacheDir / "modified.txt";
    if (fs::exists(modtimesFile))
        loadModTimes(modtimesFile);
}

void flushCaches(std::string outDir) {
    fs::path cacheDir = fs::path(outDir) / ".cache";
    fs::create_directories(cacheDir);

    fs::path modtimesFile = cacheDir / "modified.txt";
    writeModTimes(modtimesFile);
}

void loadModTimes(std::string path) {
    std::ifstream stream(path);

    std::string line;
    while (std::getline(stream, line)) {
        int pos = line.find(":");
        std::string filename = line.substr(0, pos);
        std::string timestr = line.substr(pos+1);
        uint64_t time = std::stoull(timestr);
        LAST_MODIFIED_TIMES[filename] = time;
    }

    stream.close();
}

void writeModTimes(std::string path) {
    std::ofstream stream(path);

    for (auto& [key, time] : LAST_MODIFIED_TIMES) {
        stream << key.c_str() << ":" << time << "\n";
    }

    stream.close();
}

bool hasFileBeenUpdated(std::string path) {
    path = fs::canonical(path);
    if (LAST_MODIFIED_TIMES.count(path) == 0) return true;
    
    // https://stackoverflow.com/a/31258680/16255372
    auto rawtime = fs::last_write_time(path);
    auto rawtime_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(rawtime);
    uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(rawtime_ms.time_since_epoch()).count();

    uint64_t cachedTime = LAST_MODIFIED_TIMES[path];
    return time > cachedTime;
}

void markFileCached(std::string path) {
    path = fs::canonical(path);
    // https://stackoverflow.com/a/31258680/16255372
    auto rawtime = fs::last_write_time(path);
    auto rawtime_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(rawtime);
    uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(rawtime_ms.time_since_epoch()).count();

    LAST_MODIFIED_TIMES[path] = time;
}