#include "assets.h"
#include "platform.h"

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#include <pwd.h>
#endif

static bool exists(std::string path) {
    return access(path.c_str(), F_OK) == 0;
}

std::string resolveAssetPath(std::string path) {
    std::string dataDir = getProgramDataDir();

    // Check current directory
    if (exists(path))
        return path;

    // Local user overrides
    if (exists(dataDir + "/" + path))
        return dataDir + "/" + path;

    // POSIX default system paths
#if defined(_POSIX_VERSION) || defined(__linux) || defined(__linux__) || defined(__unix__)
    if (exists("/usr/share/openblocks/" + path))
        return "/usr/share/openblocks/" + path;

    if (exists("/app/share/openblocks/" + path))
        return "/app/share/openblocks/" + path;
#endif

    return ""; // Not found
}