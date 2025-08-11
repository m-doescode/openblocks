# Modified from QGIS' FindQScintilla.cmake by Thomas Moenicke, Larry Schaffer

add_library(QScintilla::QScintilla UNKNOWN IMPORTED)

### NECESSARY TO PREVENT staticMetaObject ERROR!!! See qscintilla.prf AKA qmake config
if(WIN32)
  add_compile_definitions(QSCINTILLA_DLL)
endif()

FIND_PATH(QSCINTILLA_INCLUDE_DIR
  NAMES Qsci/qsciglobal.h
  PATHS
    ${Qt6Core_INCLUDE_DIRS}
    $ENV{LIB_DIR}/include
    /usr/local/include
    /usr/include
    ${VCPKG_INSTALLED_DIR}/x64-windows/include
  PATH_SUFFIXES ${QSCINTILLA_PATH_SUFFIXES}
)

set(QSCINTILLA_LIBRARY_NAMES
  qscintilla2-qt6
  qscintilla2_qt6
  libqt6scintilla2
  libqscintilla2-qt6
  qt6scintilla2
  libqscintilla2-qt6.dylib
  qscintilla2
)


find_library(QSCINTILLA_LIBRARY
NAMES ${QSCINTILLA_LIBRARY_NAMES}
PATHS
  "${QT_LIBRARY_DIR}"
  $ENV{LIB_DIR}/lib
  /usr/local/lib
  /usr/local/lib/qt6
  /usr/lib
  ${VCPKG_INSTALLED_DIR}/x64-windows/lib
)

get_filename_component(QSCINTILLA_LIB_DIR ${QSCINTILLA_LIBRARY} DIRECTORY)
list(TRANSFORM QSCINTILLA_LIBRARY_NAMES APPEND ".dll" OUTPUT_VARIABLE QSCINTILLA_DLL_NAMES)

find_file(QSCINTILLA_DLLS
NAMES ${QSCINTILLA_DLL_NAMES}
PATHS
  "${QT_LIBRARY_DIR}"
  $ENV{LIB_DIR}/lib
  /usr/local/lib
  /usr/local/lib/qt6
  /usr/lib
  ${QSCINTILLA_LIB_DIR}
  ${VCPKG_INSTALLED_DIR}/x64-windows/lib
)