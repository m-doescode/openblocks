#include <clang-c/CXDiagnostic.h>
#include <clang-c/CXFile.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <filesystem>
#include "cache.h"
#include "util.h"

namespace fs = std::filesystem;

bool analyzeFirstPass(std::string path, std::string srcRoot);

std::map<std::string, std::vector<std::string>> SUBCLASSES;
std::map<std::string, std::string> SUPERCLASS;

int main(int argc, char** argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: autogen <src-root> <parse-dir> <out-dir>\n");
        return 1;
    }

    loadCaches(argv[3]);

    std::vector<std::string> headerFiles;

    // Scrape directory for header files
    for (const fs::directory_entry& file : fs::recursive_directory_iterator(argv[2])) {
        if (!file.is_regular_file()) continue; // Not a file, skip
        if (file.path().extension() != ".cpp") continue; // Not a header file, skip
        if (!hasFileBeenUpdated(file.path())) {
            fs::path relpath = fs::relative(file.path(), argv[1]);
            printf("[AUTOGEN] Skipping file %s...\n", relpath.c_str());
            continue;
        }
        markFileCached(file.path());

        headerFiles.push_back(file.path());
    }

    // First-pass: Analyze type hierarchy
    for (std::string path : headerFiles) {
        fs::path relpath = fs::relative(path, argv[1]);
        printf("[AUTOGEN] [Stage 1] Analyzing file %s...\n", relpath.c_str());
        if (!analyzeFirstPass(path, argv[1]))
            return 1;
    }

    flushCaches(argv[3]);
}

// https://clang.llvm.org/docs/LibClang.html
bool analyzeFirstPass(std::string path, std::string srcRoot) {
    const char* cargs[] = { "-x", "c++", "-I", srcRoot.c_str(), 0 };
    // THANK YOU SO MUCH THIS STACKOVERFLOW ANSWER IS SO HELPFUL
    // https://stackoverflow.com/a/59206378/16255372
    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        path.c_str(), cargs, 4,
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

    // Search for classes
    x_clang_visitChildren(cursor, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_ClassDecl) return CXChildVisit_Continue;

        std::string className = x_clang_toString(clang_getCursorDisplayName(cur));

        std::string baseClass = "";
        x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
            CXCursorKind kind = clang_getCursorKind(cur);
            if (kind != CXCursor_CXXBaseSpecifier) return CXChildVisit_Continue;

            baseClass = x_clang_toString(clang_getCursorDisplayName(cur));
            return CXChildVisit_Break;
        });

        if (baseClass != "") {
            SUPERCLASS[className] = baseClass;

            std::vector<std::string> subclasses = SUBCLASSES[baseClass];
            subclasses.push_back(className);
            SUBCLASSES[baseClass] = subclasses;
        }

        return CXChildVisit_Continue;
    });

    return true;
}