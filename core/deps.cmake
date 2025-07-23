
include(CPM)

CPMAddPackage("gh:Perlmint/glew-cmake#glew-cmake-2.2.0")
CPMAddPackage("gh:g-truc/glm#1.0.1")
CPMAddPackage(NAME reactphysics3d GITHUB_REPOSITORY "DanielChappuis/reactphysics3d" VERSION 0.10.2 PATCHES ${CMAKE_SOURCE_DIR}/patches/std_chrono.patch)
# https://github.com/StereoKit/StereoKit/blob/0be056efebcee5e58ad1438f4cf6dfdb942f6cf9/CMakeLists.txt#L205
set_property(TARGET reactphysics3d PROPERTY POSITION_INDEPENDENT_CODE ON)
CPMAddPackage("gh:zeux/pugixml@1.15")

CPMAddPackage(
    NAME freetype
    GIT_REPOSITORY https://github.com/aseprite/freetype2.git
    GIT_TAG VER-2-10-0
    VERSION 2.10.0
)

if (freetype_ADDED)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

CPMAddPackage("gh:nothings/stb#8cfb1605c02aee9fb6eb5d8ea559017745bd9a16") # 2.14
CPMAddPackage("gh:WohlSoft/LuaJIT#a5da8f4a31972b74254f00969111b8b7a07cf584") # v2.1
set(LUAJIT_INCLUDE_DIRS ${LuaJIT_SOURCE_DIR}/src)