#include <clang-c/CXDiagnostic.h>
#include <clang-c/CXFile.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <cstdio>
#include <fstream>
#include <filesystem>

#include "object/analysis.h"
#include "object/codegen.h"

// namespace data {
//     #include "data/analysis.h"
//     #include "data/codegen.h"
// }

namespace fs = std::filesystem;

int processObject(fs::path srcRoot, fs::path srcPath, fs::path outPath) {
    object::AnalysisState state;

    fs::path relpath = fs::relative(srcPath, srcRoot);
    printf("[AUTOGEN] Processing file %s...\n", relpath.c_str());
    object::analyzeClasses(srcPath, srcRoot, &state);

    fs::create_directories(outPath.parent_path()); // Make sure generated dir exists before we try writing to it

    printf("[AUTOGEN] Generating file %s...\n", relpath.c_str());
    std::ofstream outStream(outPath);

    for (auto& [_, clazz] : state.classes) {
        object::writeCodeForClass(outStream, clazz);
    }

    outStream.close();

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: autogen <object|data> <src-root> <src-file> <out-dir>\n");
        return 1;
    }

    std::string codeType = argv[1];

    fs::path srcRoot = argv[2];
    fs::path srcPath = argv[3];
    fs::path outPath = argv[4];

    if (codeType == "object") {
        return processObject(srcRoot, srcPath, outPath);
    } else if (codeType == "data") {
        return processObject(srcRoot, srcPath, outPath);
    }

    fprintf(stderr, "Unknown class type '%s'\n", codeType.c_str());
    return -1;
}