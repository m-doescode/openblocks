#include <clang-c/CXDiagnostic.h>
#include <clang-c/CXFile.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <cstdio>
#include <string>
#include <vector>
#include <filesystem>
#include "analysis.h"
#include "cache.h"

namespace fs = std::filesystem;

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
        if (file.path().extension() != ".h") continue; // Not a header file, skip
        if (!hasFileBeenUpdated(file.path())) {
            fs::path relpath = fs::relative(file.path(), argv[1]);
            printf("[AUTOGEN] Skipping file %s...\n", relpath.c_str());
            continue;
        }
        markFileCached(file.path());

        headerFiles.push_back(file.path());
    }

    AnalysisState state;

    analyzeClasses("../core/src/objects/part.h", argv[1], &state);


    // for (auto& [_, clazz] : state.classes) {
    //     printf("Class: %s\n", clazz.name.c_str());
    //     if (clazz.baseClass != "")
    //         printf("==> Base class: %s\n", clazz.baseClass.c_str());
    //     if (clazz.explorerIcon != "")
    //         printf("==> Explorer icon: %s\n", clazz.explorerIcon.c_str());
    //     printf("==> Flags (%x): ", clazz.flags);
    //     if (clazz.flags & ClassFlag_Service)
    //         printf("INSTANCE_SERVICE ");
    //     if (clazz.flags & ClassFlag_NotCreatable)
    //         printf("INSTANCE_NOT_CREATABLE ");
    //     if (clazz.flags & ClassFlag_Hidden)
    //         printf("INSTANCE_HIDDEN");
    //     printf("\n");

    //     if (!clazz.properties.empty())
    //         printf("==> Properties:\n");
    //     for (auto prop : clazz.properties) {
    //         printf("====> %s (%s) (%s)\n", prop.name.c_str(), prop.fieldName.c_str(), prop.backingFieldType.c_str());
    //     }
    // }

    // First-pass: Analyze type hierarchy
    // for (std::string path : headerFiles) {
    //     fs::path relpath = fs::relative(path, argv[1]);
    //     printf("[AUTOGEN] Processing file %s...\n", relpath.c_str());
    //     analyzeClasses(path, argv[1], &state);
    // }

    flushCaches(argv[3]);
}