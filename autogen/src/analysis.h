#pragma once

#include <map>
#include <string>
#include <vector>

enum ClassFlags {
    ClassFlag_NotCreatable = 1<<0,
    ClassFlag_Service = 1<<1,
    ClassFlag_Hidden = 1<<2,
};

enum PropertyFlags {
    PropertyFlag_Hidden = 1 << 0,
    PropertyFlag_NoSave = 1 << 1,
    PropertyFlag_UnitFloat = 1 << 2,
    PropertyFlag_Readonly = 1 << 3,
};

struct PropertyAnalysis {
    std::string name;
    std::string fieldName;
    std::string backingFieldType;
    std::string onUpdateCallback;
    std::string category;
    PropertyFlags flags;
};

// https://stackoverflow.com/a/1448478/16255372
inline ClassFlags operator|(ClassFlags a, ClassFlags b) {
    return static_cast<ClassFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline PropertyFlags operator|(PropertyFlags a, PropertyFlags b) {
    return static_cast<PropertyFlags>(static_cast<int>(a) | static_cast<int>(b));
}

struct ClassAnalysis {
    std::string name;
    std::string baseClass;
    std::string headerPath;
    std::vector<PropertyAnalysis> properties;
    ClassFlags flags = (ClassFlags)0;
    std::string explorerIcon;
};

struct AnalysisState {
    std::map<std::string, ClassAnalysis> classes;
};

bool analyzeClasses(std::string path, std::string srcRoot, AnalysisState* state);