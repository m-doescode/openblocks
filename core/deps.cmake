
include(CPM)

# Some packages will build helper binaries. This keeps them out of our own build output
set (PREV_BIN_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
unset (CMAKE_RUNTIME_OUTPUT_DIRECTORY)

CPMAddPackage("gh:g-truc/glm#1.0.1")
CPMAddPackage(NAME Jolt GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics" VERSION 5.3.0 SOURCE_SUBDIR "Build")
CPMAddPackage("gh:zeux/pugixml@1.15")

CPMAddPackage(
    NAME freetype
    GIT_REPOSITORY https://github.com/aseprite/freetype2.git
    GIT_TAG VER-2-10-0
    VERSION 2.10.0
    PATCHES ${CMAKE_SOURCE_DIR}/patches/freetype_cmakever.patch
)

if (freetype_ADDED)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

CPMAddPackage("gh:nothings/stb#8cfb1605c02aee9fb6eb5d8ea559017745bd9a16") # 2.14
CPMAddPackage("gh:WohlSoft/LuaJIT#a5da8f4a31972b74254f00969111b8b7a07cf584") # v2.1
set(LUAJIT_INCLUDE_DIRS ${LuaJIT_SOURCE_DIR}/src)

CPMAddPackage("gh:mackron/miniaudio#0.11.22")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PREV_BIN_PATH})