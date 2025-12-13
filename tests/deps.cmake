
include(CPM)

CPMAddPackage("gh:catchorg/Catch2@3.8.1")

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
