#include "analysis.h"
#include "../util.h"
#include <clang-c/CXFile.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/Index.h>
#include <cstdio>
#include <optional>
#include <filesystem>

using namespace object;

namespace fs = std::filesystem;

static bool findInstanceAnnotation(CXCursor cur) {
    bool found = false;
    // Look for annotations in the class itself
    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_AnnotateAttr) return CXChildVisit_Continue;

        std::string annString = x_clang_toString(clang_getCursorDisplayName(cur));
        // if (annString == "OB::INSTANCE") found = true;
        if (annString == "OB::def_inst") found = true;

        return CXChildVisit_Break;
    });
    return found;
}

static std::string findBaseClass(CXCursor cur) {
    std::string baseClass = "";
    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_CXXBaseSpecifier) return CXChildVisit_Continue;

        baseClass = x_clang_toString(clang_getCursorDisplayName(cur));

        return CXChildVisit_Break;
    });
    return baseClass;
}

static std::string currentCategory = "";
static void processField(CXCursor cur, ClassAnalysis* state) {
    std::optional<std::string> propertyDef = findAnnotation(cur, "OB::def_prop");
    if (!propertyDef) return;

    // Update current category
    std::optional<std::string> categoryDef = findAnnotation(cur, "OB::def_prop_category");
    if (categoryDef) {
        currentCategory = parseAnnotationString(categoryDef.value())["category"];
    }

    PropertyAnalysis anly;

    auto result = parseAnnotationString(propertyDef.value());
    std::string fieldName = x_clang_toString(clang_getCursorDisplayName(cur));

    anly.name = result["name"];
    anly.fieldName = fieldName;
    anly.category = result["category"];
    anly.onUpdateCallback = result["on_update"];

    if (anly.category == "")
        anly.category = currentCategory;

    // if name field is not provided, use fieldName instead, but capitalize the first character
    if (anly.name == "") {
        anly.name = fieldName;
        anly.name[0] = std::toupper(anly.name[0]);
    }
    
    if (result.count("hidden"))
        anly.flags = anly.flags | PropertyFlags::PropertyFlag_Hidden;
    if (result.count("no_save"))
        anly.flags = anly.flags | PropertyFlags::PropertyFlag_NoSave;
    if (result.count("unit_float"))
        anly.flags = anly.flags | PropertyFlags::PropertyFlag_UnitFloat;
    if (result.count("readonly"))
        anly.flags = anly.flags | PropertyFlags::PropertyFlag_Readonly;

    CXType type = clang_getCursorType(cur);
    anly.backingFieldType = x_clang_toString(clang_getTypeSpelling(type)).c_str();

    state->properties.push_back(anly);

    // For cframe, add member fields

    std::optional<std::string> cframePositionDef = findAnnotation(cur, "OB::cframe_position_prop");
    if (cframePositionDef) {
        auto cframePosition = parseAnnotationString(cframePositionDef.value());

        PropertyAnalysis cframeProp;
        cframeProp.backingFieldType = "Vector3";
        cframeProp.fieldName = anly.fieldName;
        cframeProp.name = cframePosition["name"];
        cframeProp.category = anly.category;
        cframeProp.cframeMember = CFrameMember_Position;
        cframeProp.onUpdateCallback = anly.onUpdateCallback;
        cframeProp.flags = PropertyFlag_NoSave;

        state->properties.push_back(cframeProp);
    };

    std::optional<std::string> cframeRotationDef = findAnnotation(cur, "OB::cframe_rotation_prop");
    if (cframeRotationDef) {
        auto cframeRotation = parseAnnotationString(cframeRotationDef.value());

        PropertyAnalysis cframeProp;
        cframeProp.backingFieldType = "Vector3";
        cframeProp.fieldName = anly.fieldName;
        cframeProp.name = cframeRotation["name"];
        cframeProp.category = anly.category;
        cframeProp.cframeMember = CFrameMember_Rotation;
        cframeProp.onUpdateCallback = anly.onUpdateCallback;
        cframeProp.flags = PropertyFlag_NoSave;

        state->properties.push_back(cframeProp);
    };
}

static void processClass(CXCursor cur, AnalysisState* state, std::string className, std::string srcRoot) {
    ClassAnalysis anly;

    // Find base class
    std::string baseClass = findBaseClass(cur);

    // Find header file
    CXSourceLocation loc = clang_getCursorLocation(cur);
    CXFile file;
    unsigned line, column, off;
    clang_getFileLocation(loc, &file, &line, &column, &off);
    std::string headerName = x_clang_toString(clang_getFileName(file));
    fs::path headerPath = fs::relative(headerName, srcRoot);

    anly.name = className;
    anly.baseClass = baseClass;
    anly.headerPath = headerPath;

    // Add misc flags and options
    auto instanceDef = findAnnotation(cur, "OB::def_inst");
    auto result = parseAnnotationString(instanceDef.value());

    if (result.count("service"))
        anly.flags = anly.flags | ClassFlag_Service | ClassFlag_NotCreatable;
    if (result.count("not_creatable"))
        anly.flags = anly.flags | ClassFlag_NotCreatable;
    if (result.count("hidden"))
        anly.flags = anly.flags | ClassFlag_Hidden;

    anly.abstract = result.count("abstract") > 0;

    anly.explorerIcon = result["explorer_icon"];
    
    // Find annotated fields
    currentCategory = "";
    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_FieldDecl) return CXChildVisit_Continue;
        
        processField(cur, &anly);

        return CXChildVisit_Continue;
    });

    state->classes[className] = anly;
}

// https://clang.llvm.org/docs/LibClang.html
bool object::analyzeClasses(CXCursor cursor, std::string srcRoot, AnalysisState* state) {
    // Search for classes
    x_clang_visitChildren(cursor, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_ClassDecl) return CXChildVisit_Continue;

        CXSourceLocation loc = clang_getCursorLocation(cur);
        if (!clang_Location_isFromMainFile(loc)) return CXChildVisit_Continue; // This class is not from this header. Skip

        std::string className = x_clang_toString(clang_getCursorDisplayName(cur));
        // Forward-decls can slip through the cracks, this prevents that, but also allows us to filter non-instance classes in the src/objects directory
        if (!findInstanceAnnotation(cur)) return CXChildVisit_Continue; // Class is not "primary" declaration/is not instance, skip
        if (state->classes.count(className) > 0) return CXChildVisit_Continue; // Class has already been analyzed, skip...

        processClass(cur, state, className, srcRoot);

        return CXChildVisit_Continue;
    });

    return true;
}