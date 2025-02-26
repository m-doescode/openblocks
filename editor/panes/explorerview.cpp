#include "explorerview.h"
#include "explorermodel.h"
#include "common.h"
#include "objects/base/instance.h"
#include "objects/workspace.h"
#include "qabstractitemmodel.h"
#include "qaction.h"
#include "qnamespace.h"
#include <qitemselectionmodel.h>

ExplorerView::ExplorerView(QWidget* parent):
    QTreeView(parent),
    model(ExplorerModel(std::dynamic_pointer_cast<Instance>(dataModel))) {

    this->setModel(&model);
    // Disabling the root decoration will cause the expand/collapse chevrons to be hidden too, we don't want that
    // https://stackoverflow.com/a/4687016/16255372
    // this->setRootIsDecorated(false);
    // The branches can be customized like this if you want:
    // this->setStyleSheet(QString("QTreeView::branch { border: none; }"));
    this->setDragDropMode(QAbstractItemView::InternalMove);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setDragEnabled(true);
    this->setAcceptDrops(true);
    this->setDropIndicatorShown(true);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    // Expand workspace
    this->expand(model.ObjectToIndex(workspace()));

    connect(this, &QTreeView::customContextMenuRequested, this, [&](const QPoint& point) {
        QModelIndex index = this->indexAt(point);
        contextMenu.exec(this->viewport()->mapToGlobal(point));
    });

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, [&](const QItemSelection &selected, const QItemSelection &deselected) {
        std::vector<InstanceRefWeak> selectedInstances;
        selectedInstances.reserve(selected.count()); // This doesn't reserve everything, but should enhance things anyway

        for (auto range : selected) {
            for (auto index : range.indexes()) {
                selectedInstances.push_back(reinterpret_cast<Instance*>(index.internalPointer())->weak_from_this());
            }
        }

        ::setSelection(selectedInstances, true);
    });

    addSelectionListener([&](auto oldSelection, auto newSelection, bool fromExplorer) {
        // It's from us, ignore it.
        if (fromExplorer) return;

        this->clearSelection();
        for (InstanceRefWeak inst : newSelection) {
            if (inst.expired()) continue;
            QModelIndex index = this->model.ObjectToIndex(inst.lock());
            this->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::Select);
        }
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