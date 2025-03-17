#include "platform.h"

#include <filesystem>
#include "panic.h"

// GNU/Linux implementation
#if defined(_POSIX_VERSION) || defined(__linux) || defined(__linux__)

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstdio>
#include <cstdlib>

std::string getProgramDataDir() {
    // https://stackoverflow.com/a/26696759/16255372

    const char *homedir;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    return std::string(homedir) + "/openblocks";
}

void displayErrorMessage(std::string message) {
    fprintf(stderr, "%s\n", message.c_str());
}

#endif // GNU/Linux

// Windows implementation
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <cstdio>
#include <cstdlib>
#include <shlobj.h>
#include <winuser.h>

std::string getProgramDataDir() {
    CHAR localAppData[MAX_PATH];
    int status = SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData);
    if (status != 0) {
        displayErrorMessage("Failed to find local appdata folder");
        panic();
    }
    return std::string(localAppData) + "/openblocks";
}

void displayErrorMessage(std::string message) {
    fprintf(stderr, "%s\n", message.c_str());
    MessageBoxA(NULL, message.c_str(), "Fatal Error", MB_OK | MB_ICONERROR);
}

#endif // WIN32

std::string getProgramLogsDir() {
    return getProgramDataDir() + "/logs";
}

void initProgramDataDir() {
    std::filesystem::create_directories(getProgramDataDir());
}

void initProgramLogsDir() {
    std::filesystem::create_directories(getProgramLogsDir());
}
