# Modified from QGIS' FindQScintilla.cmake by Thomas Moenicke, Larry Schaffer

add_library(Clang::Clang UNKNOWN IMPORTED)

FIND_PATH(CLANG_INCLUDE_DIR
  NAMES clang-c/Index.h
  PATHS
    $ENV{LIB_DIR}/include
    /usr/local/include
    /usr/include
    ${VCPKG_INSTALLED_DIR}/x64-windows/include
    "C:/Program Files/LLVM/include"
  PATH_SUFFIXES ${CLANG_PATH_SUFFIXES}
)

set(CLANG_LIBRARY_NAMES
  libclang
  clang
)

find_library(CLANG_LIBRARY
NAMES ${CLANG_LIBRARY_NAMES}
PATHS
  $ENV{LIB_DIR}/lib
  /usr/local/lib
  /usr/lib
  ${VCPKG_INSTALLED_DIR}/x64-windows/lib
  "C:/Program Files/LLVM/lib"
)

get_filename_component(CLANG_LIB_DIR ${CLANG_LIBRARY} DIRECTORY)
list(TRANSFORM CLANG_LIBRARY_NAMES APPEND ".dll" OUTPUT_VARIABLE CLANG_DLL_NAMES)

find_file(CLANG_DLLS
NAMES ${CLANG_DLL_NAMES}
PATHS
  $ENV{LIB_DIR}/bin
  /usr/local/bin
  /usr/bin
  ${VCPKG_INSTALLED_DIR}/x64-windows/bin
  "C:/Program Files/LLVM/bin"
)