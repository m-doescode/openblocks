set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(QT_VERSION_MAJOR 6)
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets REQUIRED)
find_package(QScintilla6 REQUIRED)

include(${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake)

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

qt_add_executable(editor MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_include_directories(editor PRIVATE . "${CMAKE_SOURCE_DIR}/external/glad")
target_link_libraries(editor PRIVATE openblocks Qt6::Widgets Qt6::OpenGLWidgets ${QSCINTILLA_LIBRARY} miniaudio QCursorConstraints)

set_target_properties(editor PROPERTIES
    WIN32_EXECUTABLE ON
)

# Copy Qt files
if (WIN32)
    #include("${QT_DEPLOY_SUPPORT}")

    # Copy assets
    add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:editor>/assets)

    # Copy over QScintilla DLLs
    # TODO: Use a better approach?
    add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${QSCINTILLA_DLLS} $<TARGET_FILE_DIR:editor>
    )

    set(WINDEPLOYQT_OPTIONS
        --dir $<TARGET_FILE_DIR:editor>
        --translations en
        --no-compiler-runtime
        --no-opengl-sw # No sense adding opengl-sw given that hardware acceleration is necessary, anyway
        --no-system-d3d-compiler
        --plugindir $<TARGET_FILE_DIR:editor>/qtplugins
        # Also don't want to clutter with plugins, add only needed ones
    )

    # We split these into two commands because
    # we might build a debug binary against a release qscintilla library
    # TODO: Add other translations
    add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:editor>
                ${WINDEPLOYQT_OPTIONS}
    )

    # Copy QScintilla dependencies too
    add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${WINDEPLOYQT_EXECUTABLE} ${QSCINTILLA_DLLS}
                ${WINDEPLOYQT_OPTIONS}
    )

    # Copy qt.conf to override default plugins location
    add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qt.conf $<TARGET_FILE_DIR:editor>/qt.conf
    )
endif()

qt_finalize_executable(editor)

install(TARGETS editor DESTINATION bin)