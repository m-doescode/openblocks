# Modified from QGIS' FindQScintilla.cmake by Thomas Moenicke, Larry Schaffer

add_library(QScintilla::QScintilla UNKNOWN IMPORTED)

### NECESSARY TO PREVENT staticMetaObject ERROR!!! See qscintilla.prf AKA qmake config
if(WIN32)
  add_compile_definitions(QSCINTILLA_DLL)
endif()

FIND_PATH(QSCINTILLA_INCLUDE_DIR
  NAMES Qsci/qsciglobal.h
  PATHS
    ${Qt${QT_VERSION_MAJOR}Core_INCLUDE_DIRS}
    $ENV{LIB_DIR}/include
    /usr/local/include
    /usr/include
    ${VCPKG_INSTALLED_DIR}/x64-windows/include
  PATH_SUFFIXES ${QSCINTILLA_PATH_SUFFIXES}
)

set(QSCINTILLA_LIBRARY_NAMES
  qscintilla2-qt${QT_VERSION_MAJOR}
  qscintilla2_qt${QT_VERSION_MAJOR}
  libqt${QT_VERSION_MAJOR}scintilla2
  libqscintilla2-qt${QT_VERSION_MAJOR}
  qt${QT_VERSION_MAJOR}scintilla2
  libqscintilla2-qt${QT_VERSION_MAJOR}.dylib
  qscintilla2
)


find_library(QSCINTILLA_LIBRARY
NAMES ${QSCINTILLA_LIBRARY_NAMES}
PATHS
  "${QT_LIBRARY_DIR}"
  $ENV{LIB_DIR}/lib
  /usr/local/lib
  /usr/local/lib/qt${QT_VERSION_MAJOR}
  /usr/lib
  ${VCPKG_INSTALLED_DIR}/x64-windows/lib
)

list(TRANSFORM QSCINTILLA_LIBRARY_NAMES APPEND ".dll" OUTPUT_VARIABLE QSCINTILLA_DLL_NAMES)

find_file(QSCINTILLA_DLLS
NAMES ${QSCINTILLA_DLL_NAMES}
PATHS
  "${QT_LIBRARY_DIR}"
  $ENV{LIB_DIR}/lib
  /usr/local/lib
  /usr/local/lib/qt${QT_VERSION_MAJOR}
  /usr/lib
  ${VCPKG_INSTALLED_DIR}/x64-windows/lib
)