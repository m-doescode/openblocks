cmake_minimum_required(VERSION 3.30.0)

project(editor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia LinguistTools)
find_package(QScintilla REQUIRED)

set(TS_FILES editor_en_US.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        mainglwidget.h
        mainglwidget.cpp
        placedocument.h
        placedocument.cpp
        undohistory.h
        undohistory.cpp
        panes/explorerview.h
        panes/explorerview.cpp
        panes/explorermodel.h
        panes/explorermodel.cpp
        panes/propertiesview.h
        panes/propertiesview.cpp
        panes/outputtextview.h
        panes/outputtextview.cpp
        script/scriptdocument.h
        script/scriptdocument.cpp
        script/commandedit.h
        script/commandedit.cpp
        aboutdialog.ui
        aboutdialog.h
        aboutdialog.cpp
        editor.qrc
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(editor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET editor APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(editor SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(editor
            ${PROJECT_SOURCES}
            mainglwidget.h mainglwidget.cpp
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(editor PUBLIC "../core/src" "../include" ${QSCINTILLA_INCLUDE_DIR})
target_link_libraries(editor PRIVATE openblocks Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia ${QSCINTILLA_LIBRARY})
add_dependencies(editor openblocks)

# Qt6 does not include QOpenGLWidgets as part of Widgets base anymore, so
# we have to include it manually
if (${QT_VERSION} GREATER_EQUAL 6)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS OpenGL OpenGLWidgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGL OpenGLWidgets)

    target_include_directories(editor PUBLIC Qt6::OpenGL Qt6::OpenGLWidgets)
    target_link_libraries(editor PRIVATE Qt6::OpenGL Qt6::OpenGLWidgets)
endif()

# Copy assets
add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/assets
                $<TARGET_FILE_DIR:editor>/assets)

# Copy Qt files
if (WIN32)
    #include("${QT_DEPLOY_SUPPORT}")

    # Copy over QScintilla DLLs
    # TODO: Use a better approach?
    add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${QSCINTILLA_DLLS} $<TARGET_FILE_DIR:editor>
    )

    # TODO: Add other translations
    add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:editor> ${QSCINTILLA_DLLS} --release --dir $<TARGET_FILE_DIR:editor> --translations en --no-compiler-runtime --no-opengl-sw --no-system-d3d-compiler --plugindir $<TARGET_FILE_DIR:editor>/qtplugins
    )
    # No sense adding opengl-sw given that hardware acceleration is necessary, anyway
    # Also don't want to clutter with plugins, add only needed ones


    # Copy qt.conf to override default plugins location
    add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qt.conf $<TARGET_FILE_DIR:editor>/qt.conf
    )
endif ()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qtbasic)
endif()
set_target_properties(editor PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS editor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES $<TARGET_RUNTIME_DLLS:editor> TYPE BIN)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(editor)
endif()