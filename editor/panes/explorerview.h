#pragma once

#include "objects/base/instance.h"
#include "objects/part.h"
#include "qevent.h"
#include "qmenu.h"
#include "qnamespace.h"
#include "qtreeview.h"
#include <QOpenGLWidget>
#include <QWidget>
#include <memory>
#include "explorermodel.h"

class Ui_MainWindow;

class ExplorerView : public QTreeView {
public:
    ExplorerView(QWidget* parent = nullptr);
    ~ExplorerView() override;

    void keyPressEvent(QKeyEvent*) override;
    // void dropEvent(QDropEvent*) override;
private:
    ExplorerModel model;
    QMenu contextMenu;

    // TODO: Move these to a separate top-level namespace so these can be
    // accessed from multiple locations
    QAction* actionDelete;
    QAction* actionCopy;
    QAction* actionCut;
    QAction* actionPaste;
    QAction* actionPasteInto;
    QAction* actionSelectChildren;

    void buildContextMenu();
};