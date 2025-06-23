#pragma once

#include "panes/explorermodel.h"
#include <memory>
#include <qmenu.h>
#include <qtreeview.h>

class Ui_MainWindow;

class ExplorerView : public QTreeView {
    bool handlingSelectionUpdate = false;
public:
    ExplorerView(QWidget* parent = nullptr);
    ~ExplorerView() override;

    void keyPressEvent(QKeyEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    // void dropEvent(QDropEvent*) override;

    void buildContextMenu();
    void updateRoot(std::shared_ptr<Instance> newRoot);

    void setSelectedObjects(std::vector<std::shared_ptr<Instance>> selection);
private:
    ExplorerModel model;
    QMenu contextMenu;
};