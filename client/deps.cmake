# Declare/fetch packages

include(FetchContent)
FetchContent_Declare(
    glfw3
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw3)

# Find/include packages