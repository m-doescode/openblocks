#include "analysis.h"
#include "util.h"
#include <clang-c/CXFile.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/Index.h>
#include <cstdio>
#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

// Very simple parser
// Example format:
//  name="Hello!", world=Test, read_only
// Result:
//  "name": "Hello!", "world": "Test", "read_only": ""
std::map<std::string, std::string> parseAnnotationString(std::string src) {
    std::map<std::string, std::string> result;

    std::string currentIdent = "";
    std::string currentValue = "";
    int stage = 0;
    bool quoted = false;

    int i = 0;
    for (; i < src.length(); i++) {
        if (src[i] == ' ' && (stage != 2 || !quoted)) continue; // Ignore spaces if not in stage 2 and quoted
        if (src[i] == ',' && stage == 0) continue; // Let empty commas slip by
        if (stage < 2 && (src[i] >= 'a' && src[i] <= 'z' || src[i] >= 'A' && src[i] <= 'Z' || src[i] >= '0' && src[i] <= '9' || src[i] == '_')) {
            currentIdent += src[i];
            stage = 1;
            continue;
        }
        if (stage == 1 && src[i] == '=') { // What follows is a value
            stage = 2;
            continue;
        }
        if (stage == 1 && src[i] == ',') { // Value-less key
            stage = 0;
            result[currentIdent] = "";
            currentIdent = "";
            continue;
        }
        if (stage == 2 && quoted && src[i] == '"') { // Close a quoted string
            quoted = false;
            continue;
        }
        if (stage == 2 && !quoted && src[i] == '"') { // Start a quoted string
            quoted = true;
            continue;
        }
        if (stage == 2 && !quoted && (src[i] == ' ' || src[i] == ',')) { // Terminate the string
            stage = 0;
            result[currentIdent] = currentValue;
            currentIdent = "";
            currentValue = "";
            continue;
        }
        if (stage == 2) { // Otherwise if in stage 2, simply add the character
            currentValue += src[i];
            continue;
        }
        fprintf(stderr, "Unexpected symbol: %c at index %d\n", src[i], i);
        fprintf(stderr, "\t%s\n", src.c_str());
        fprintf(stderr, "\t%s^\n", i > 0 ? std::string(i, '~').c_str() : "");
        abort();
    }

    // Add the remaining value
    if (stage == 1) {
        result[currentIdent] = "";
    } else if (stage == 2) {
        result[currentIdent] = currentValue;
    }

    return result;
}


bool findInstanceAnnotation(CXCursor cur) {
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

std::optional<std::string> findAnnotation(CXCursor cur, std::string annotationName) {
    std::optional<std::string> ret = std::nullopt;

    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_AnnotateAttr) return CXChildVisit_Continue;

        std::string annString = x_clang_toString(clang_getCursorDisplayName(cur));
        if (annString != annotationName) return CXChildVisit_Continue;

        // Look inside for a StringLiteral

        x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
            CXCursorKind kind = clang_getCursorKind(cur);
            // if (kind != CXCursor_StringLiteral) return CXChildVisit_Recurse;
            // String literals cannot be parsed as CXCursor_StringLiteral. I don't know why.
            if (kind != CXCursor_UnexposedExpr) return CXChildVisit_Recurse;

            // https://stackoverflow.com/a/63859988/16255372
            auto res = clang_Cursor_Evaluate(cur);
            ret = clang_EvalResult_getAsStr(res);
            clang_EvalResult_dispose(res);

            return CXChildVisit_Break;
        });

        return CXChildVisit_Break;
    });
    return ret;
}

std::string findBaseClass(CXCursor cur) {
    std::string baseClass = "";
    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_CXXBaseSpecifier) return CXChildVisit_Continue;

        baseClass = x_clang_toString(clang_getCursorDisplayName(cur));

        return CXChildVisit_Break;
    });
    return baseClass;
}

void processField(CXCursor cur, ClassAnalysis* state) {
    std::optional<std::string> propertyDef = findAnnotation(cur, "OB::def_prop");
    if (!propertyDef) return;

    PropertyAnalysis anly;

    auto result = parseAnnotationString(propertyDef.value());
    std::string fieldName = x_clang_toString(clang_getCursorDisplayName(cur));

    anly.name = result["name"];
    anly.fieldName = fieldName;
    anly.category = result["category"];
    anly.onUpdateCallback = result["on_update"];
    
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
        cframeProp.backingFieldType = anly.backingFieldType;
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
        cframeProp.backingFieldType = anly.backingFieldType;
        cframeProp.fieldName = anly.fieldName;
        cframeProp.name = cframeRotation["name"];
        cframeProp.category = anly.category;
        cframeProp.cframeMember = CFrameMember_Rotation;
        cframeProp.onUpdateCallback = anly.onUpdateCallback;
        cframeProp.flags = PropertyFlag_NoSave;

        state->properties.push_back(cframeProp);
    };
}

void processClass(CXCursor cur, AnalysisState* state, std::string className, std::string srcRoot) {
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
    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_FieldDecl) return CXChildVisit_Continue;
        
        processField(cur, &anly);

        return CXChildVisit_Continue;
    });

    state->classes[className] = anly;
}

// https://clang.llvm.org/docs/LibClang.html
bool analyzeClasses(std::string path, std::string srcRoot, AnalysisState* state) {
    const char* cargs[] = { "-x", "c++", "-I", srcRoot.c_str(), "-D__AUTOGEN__", 0 };
    // THANK YOU SO MUCH THIS STACKOVERFLOW ANSWER IS SO HELPFUL
    // https://stackoverflow.com/a/59206378/16255372
    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        path.c_str(), cargs, 5,
        nullptr, 0,
        CXTranslationUnit_None);

    if (!unit) {
        fprintf(stderr, "Failed to parse file\n");
        return 1;
    }

    // Print errors
    int ndiags = clang_getNumDiagnostics(unit);
    for (int i = 0; i < ndiags; i++) {
        CXDiagnostic diag = clang_getDiagnostic(unit, i);
        CXString str = clang_formatDiagnostic(diag, 0);
        fprintf(stderr, "diag: %s\n", clang_getCString(str));

        clang_disposeString(str);
        clang_disposeDiagnostic(diag);
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);

    bool flag = false;
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