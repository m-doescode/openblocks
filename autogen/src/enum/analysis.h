#pragma once

#include <clang-c/Index.h>
#include <map>
#include <string>
#include <vector>

namespace enum_ {

struct EnumEntry {
    std::string name;
    int value;
};

struct EnumAnalysis {
    std::string name;
    std::vector<EnumEntry> entries;
};

struct AnalysisState {
    std::map<std::string, EnumAnalysis> classes;
};

bool analyzeClasses(CXCursor cursor, std::string srcRoot, AnalysisState* state);

}