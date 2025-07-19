#pragma once

// Allows files to read the version of the current build from git
// https://jonathanhamberg.com/post/cmake-embedding-git-hash/

extern const char* BUILD_COMMIT_HASH; // Commit hash of the current build
extern const char* BUILD_VERSION; // Short form of the build version v1.2.3
extern const char* BUILD_VERSION_LONG; // Long form of the build version v1.2.3-12-g1234567