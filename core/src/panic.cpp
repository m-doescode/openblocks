#include "panic.h"

#include "logger.h"
#include "platform.h"

bool trySafeAbort = false;
void panic() {
    // We've already been here, safe aborting has failed.
    if (trySafeAbort)
        abort();
    trySafeAbort = true;

#ifdef NDEBUG
    displayErrorMessage(std::string("A fatal error has occurred and Openblocks had to shut down.\n\
The currently open document will be attempted to be saved, and logs will be written to " + getProgramLogsDir()));
#endif

    // Finalize logger
    Logger::finish();

    // TODO: Autosave document

    abort();
}
