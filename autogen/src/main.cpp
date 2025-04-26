#include <clang-c/CXDiagnostic.h>
#include <clang-c/CXFile.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "analysis.h"
#include "cache.h"
#include "codegen.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: autogen <src-root> <src-file> <out-dir>\n");
        return 1;
    }

    AnalysisState state;

    fs::path srcRoot = argv[1];
    fs::path srcPath = argv[2];
    fs::path outPath = argv[3];

    fs::path relpath = fs::relative(srcPath, srcRoot);
    printf("[AUTOGEN] Processing file %s...\n", relpath.c_str());
    analyzeClasses(srcPath, srcRoot, &state);

    fs::create_directories(outPath.parent_path()); // Make sure generated dir exists before we try writing to it

    if (state.classes.empty())
        return 0;

    printf("[AUTOGEN] Generating file %s...\n", relpath.c_str());
    std::ofstream outStream(outPath);

    for (auto& [_, clazz] : state.classes) {
        writeCodeForClass(outStream, clazz);
    }

    outStream.close();

    return 0;
}