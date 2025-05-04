#pragma once

#include <clang-c/Index.h>
#include <map>
#include <string>
#include <vector>

namespace object {

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

enum CFrameMember {
    CFrameMember_None, // Not part of CFrame
    CFrameMember_Position,
    CFrameMember_Rotation
};

struct PropertyAnalysis {
    std::string name;
    std::string fieldName;
    CFrameMember cframeMember = CFrameMember_None; // for cframe_position_prop etc.
    std::string backingFieldType;
    std::string onUpdateCallback;
    std::string category;
    PropertyFlags flags = (PropertyFlags)0;
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
    bool abstract = false;
    std::vector<PropertyAnalysis> properties;
    ClassFlags flags = (ClassFlags)0;
    std::string explorerIcon;
};

struct AnalysisState {
    std::map<std::string, ClassAnalysis> classes;
};

bool analyzeClasses(CXCursor cursor, std::string srcRoot, AnalysisState* state);

}