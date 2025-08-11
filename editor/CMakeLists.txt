set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia OpenGLWidgets REQUIRED)
find_package(QScintilla6 REQUIRED)

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

qt_add_executable(editor ${PROJECT_SOURCES})
target_include_directories(editor PRIVATE .)
target_link_libraries(editor PRIVATE openblocks Qt6::Widgets Qt6::Multimedia Qt6::OpenGLWidgets ${QSCINTILLA_LIBRARY})