#pragma once

#include <map>
#include <string>
#include <vector>

struct PropertyAnalysis {
    std::string name;
    std::string fieldName;
    std::string backingFieldType;
};

struct ClassAnalysis {
    std::string name;
    std::string baseClass;
    std::vector<PropertyAnalysis> properties;
};

struct AnalysisState {
    std::map<std::string, ClassAnalysis> classes;
};

bool analyzeClasses(std::string path, std::string srcRoot, AnalysisState* state);