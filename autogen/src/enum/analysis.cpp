#include "analysis.h"
#include "../util.h"
#include <clang-c/CXFile.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/Index.h>
#include <cstdio>
#include <optional>

using namespace enum_;

static int findEnumEntryValue(CXCursor cur, bool* success = nullptr) {
    int ret = -1;
    if (success != nullptr) *success = false;
    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent)  {
        CXCursorKind kind = clang_getCursorKind(cur);

        if (kind != CXCursor_IntegerLiteral) return CXChildVisit_Recurse;

        // https://stackoverflow.com/a/63859988/16255372
        auto res = clang_Cursor_Evaluate(cur);
        ret = clang_EvalResult_getAsInt(res);
        clang_EvalResult_dispose(res);

        if (success != nullptr) *success = true;
        return CXChildVisit_Break;
    });

    return ret;
}

static void processEnumEntry(CXCursor cur, EnumAnalysis* state, int* lastValue) {
    EnumEntry anly;

    std::string name = x_clang_toString(clang_getCursorSpelling(cur));
    bool success;
    int value = findEnumEntryValue(cur, &success);

    // Default to lastValue + 1
    if (!success) value = ++(*lastValue);
    *lastValue = value;
    
    anly.name = name;
    anly.value = value;

    state->entries.push_back(anly);
}

static void processEnum(CXCursor cur, AnalysisState* state, std::string className, std::string srcRoot) {
    EnumAnalysis anly;

    anly.name = className;

    int lastValue = -1;
    
    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);

        if (kind == CXCursor_EnumConstantDecl) {
            processEnumEntry(cur, &anly, &lastValue);
        }

        return CXChildVisit_Continue;
    });

    state->classes[className] = anly;
}

bool enum_::analyzeClasses(CXCursor cursor, std::string srcRoot, AnalysisState* state) {
    // Search for classes
    x_clang_visitChildren(cursor, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind == CXCursor_Namespace) return CXChildVisit_Recurse;
        if (kind != CXCursor_EnumDecl) return CXChildVisit_Continue;

        CXSourceLocation loc = clang_getCursorLocation(cur);
        if (!clang_Location_isFromMainFile(loc)) return CXChildVisit_Continue; // This class is not from this header. Skip

        std::string className = x_clang_toString(clang_getCursorDisplayName(cur));
        // Forward-decls can slip through the cracks, this prevents that, but also allows us to filter non-instance classes in the src/objects directory
        if (!findAnnotation(cur, "OB::def_enum")) return CXChildVisit_Continue; // Class is not "primary" declaration/is not instance, skip
        if (state->classes.count(className) > 0) return CXChildVisit_Continue; // Class has already been analyzed, skip...

        processEnum(cur, state, className, srcRoot);

        return CXChildVisit_Continue;
    });

    return true;
}