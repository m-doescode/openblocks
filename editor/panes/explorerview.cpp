#include "explorerview.h"
#include "explorermodel.h"
#include "common.h"
#include "qaction.h"
#include "qnamespace.h"

ExplorerView::ExplorerView(QWidget* parent):
    QTreeView(parent),
    model(ExplorerModel(std::dynamic_pointer_cast<Instance>(workspace))) {

    this->setModel(&model);
    this->setRootIsDecorated(false);
    this->setDragDropMode(QAbstractItemView::InternalMove);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setDragEnabled(true);
    this->setAcceptDrops(true);
    this->setDropIndicatorShown(true);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QTreeView::customContextMenuRequested, this, [&](const QPoint& point) {
        QModelIndex index = this->indexAt(point);
        contextMenu.exec(this->viewport()->mapToGlobal(point));
    });

    buildContextMenu();
}

ExplorerView::~ExplorerView() {
}

void ExplorerView::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Delete:
        actionDelete->trigger();
        break;
    }
}

void ExplorerView::buildContextMenu() {
    // This will leak memory. Anyway...
    contextMenu.addAction(this->actionDelete = new QAction(QIcon("assets/icons/editor/delete"), "Delete"));

    connect(actionDelete, &QAction::triggered, this, [&]() {
        QModelIndexList selectedIndexes = this->selectionModel()->selectedIndexes();
        for (QModelIndex index : selectedIndexes) {
            model.fromIndex(index)->SetParent(std::nullopt);
        }
    });
}