
# https://jonathanhamberg.com/post/cmake-embedding-git-hash/

# Detect current version from git
execute_process(
    COMMAND git rev-parse HEAD
    OUTPUT_VARIABLE GIT_COMMIT_HASH RESULT_VARIABLE GIT_RESULT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

execute_process(
    COMMAND git describe --abbrev=0
    OUTPUT_VARIABLE GIT_VERSION RESULT_VARIABLE GIT_RESULT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    
execute_process(
    COMMAND git describe --dirty
    OUTPUT_VARIABLE GIT_VERSION_LONG RESULT_VARIABLE GIT_RESULT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

# For some reason, CMake sets CMAKE_*_DIR all to be CMAKE_CURRENT_BINARY_DIR
# so we have to bypass this by passing in custom "orig_" variables
if (NOT GIT_STATE_WITHIN)
    # Re-run this target always so that the version can be checked
    add_custom_target(recheck_git_version ALL COMMAND ${CMAKE_COMMAND}
        -DGIT_STATE_WITHIN=1
        -DORIG_BINARY_DIR=${CMAKE_BINARY_DIR}
        -DORIG_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DORIG_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DORIG_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_MODULE_PATH}/gitversion.cmake
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/src/version.cpp
    )
else ()
    # # Set defaults if the git commands fail
    if (NOT GIT_RESULT EQUAL 0)
        set(GIT_COMMIT_HASH "unknown")
        set(GIT_VERSION "unknown")
        set(GIT_VERSION_LONG "unknown")
    endif ()

    # configure_file only touches the file if it has been changed, so no caching is necessary
    configure_file(${ORIG_CURRENT_SOURCE_DIR}/src/version.cpp.in ${ORIG_CURRENT_BINARY_DIR}/src/version.cpp @ONLY)
endif ()
