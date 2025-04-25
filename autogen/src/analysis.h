#pragma once

#include <map>
#include <string>
#include <vector>

struct PropertyAnalysis {
    std::string name;
    std::string fieldName;
    std::string backingFieldType;
};

enum ClassFlags {
    ClassFlag_NotCreatable = 1<<0,
    ClassFlag_Service = 1<<1,
    ClassFlag_Hidden = 1<<2,
};

// https://stackoverflow.com/a/1448478/16255372
inline ClassFlags operator|(ClassFlags a, ClassFlags b) {
    return static_cast<ClassFlags>(static_cast<int>(a) | static_cast<int>(b));
}

struct ClassAnalysis {
    std::string name;
    std::string baseClass;
    std::vector<PropertyAnalysis> properties;
    ClassFlags flags = (ClassFlags)0;
    std::string explorerIcon;
};

struct AnalysisState {
    std::map<std::string, ClassAnalysis> classes;
};

bool analyzeClasses(std::string path, std::string srcRoot, AnalysisState* state);