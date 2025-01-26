#include "explorermodel.h"
#include "objects/base/instance.h"
#include "qabstractitemmodel.h"
#include "qcontainerfwd.h"
#include "qimage.h"
#include "qnamespace.h"
#include "qobject.h"
#include "qstringview.h"
#include "qwidget.h"
#include "qmimedata.h"
#include "common.h"
#include <algorithm>
#include <cstdio>
#include <optional>
#include <vector>
#include "objects/base/instance.h"

// https://doc.qt.io/qt-6/qtwidgets-itemviews-simpletreemodel-example.html#testing-the-model

std::map<std::string, QImage> instanceIconCache;

ExplorerModel::ExplorerModel(InstanceRef dataRoot, QWidget *parent)
    : QAbstractItemModel(parent)
    , rootItem(dataRoot) {
    // TODO: Don't use lambdas and handlers like that
    hierarchyPreUpdateHandler = [&](InstanceRef object, std::optional<InstanceRef> oldParent, std::optional<InstanceRef> newParent) {
        if (oldParent.has_value()) {
            auto children = oldParent.value()->GetChildren();
            size_t idx = std::find(children.begin(), children.end(), object) - children.begin();
            beginRemoveRows(toIndex(oldParent.value()), idx, idx);
        }

        if (newParent.has_value()) {
            size_t size = newParent.value()->GetChildren().size();
            beginInsertRows(toIndex(newParent.value()), size, size);
        } else {
            // TODO:
        }
    };

    hierarchyPostUpdateHandler = [&](InstanceRef object, std::optional<InstanceRef> oldParent, std::optional<InstanceRef> newParent) {
        if (newParent.has_value()) endInsertRows();
        if (oldParent.has_value()) endRemoveRows();
    };
}

ExplorerModel::~ExplorerModel() = default;

QModelIndex ExplorerModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return {};

    Instance* parentItem = parent.isValid()
        ? static_cast<Instance*>(parent.internalPointer())
        : rootItem.get();

    if (parentItem->GetChildren().size() >= row)
        return createIndex(row, column, parentItem->GetChildren()[row].get());
    return {};
}

QModelIndex ExplorerModel::toIndex(InstanceRef item) {
    if (item == rootItem)
        return {};

    InstanceRef parentItem = item->GetParent().value();
    // Check above ensures this item is not root, so value() must be valid
    for (int i = 0; i < parentItem->GetChildren().size(); i++)
        if (parentItem->GetChildren()[i] == item)
            return createIndex(i, 0, item.get());
    return QModelIndex{};
}

QModelIndex ExplorerModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return {};

    Instance* childItem = static_cast<Instance*>(index.internalPointer());
    // NORISK: The parent must exist if the child was obtained from it during this frame
    InstanceRef parentItem = childItem->GetParent().value();

    if (parentItem == rootItem)
        return {};

    // Check above ensures this item is not root, so value() must be valid
    InstanceRef parentParent = parentItem->GetParent().value();
    for (int i = 0; i < parentParent->GetChildren().size(); i++)
        if (parentParent->GetChildren()[i] == parentItem)
            return createIndex(i, 0, parentItem.get());
    return QModelIndex{};
}

int ExplorerModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return 0;

    Instance* parentItem = parent.isValid()
        ? static_cast<Instance*>(parent.internalPointer())
        : rootItem.get();

    return parentItem->GetChildren().size();
}

int ExplorerModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

QVariant ExplorerModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return {};

    Instance *item = static_cast<Instance*>(index.internalPointer());

    switch (role) {
        case Qt::DisplayRole:
            return QString::fromStdString(item->name);
        case Qt::DecorationRole:
            return iconOf(item->GetClass());
    }
    return {};
}

bool ExplorerModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || role != Qt::EditRole) return false;

    Instance* inst = static_cast<Instance*>(index.internalPointer());
    inst->name = value.toString().toStdString();
    return true;
}

QVariant ExplorerModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    return QString("Idk lol \u00AF\u005C\u005F\u0028\u30C4\u0029\u005F\u002F\u00AF");
}

Qt::ItemFlags ExplorerModel::flags(const QModelIndex &index) const
{
    //return index.isValid()
    //    ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
    return index.isValid()
        ? QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled
        : Qt::NoItemFlags | Qt::ItemIsDropEnabled;
}

QImage ExplorerModel::iconOf(InstanceType* type) const {
    if (instanceIconCache.count(type->className)) return instanceIconCache[type->className];

    InstanceType* currentClass = type;
    while (currentClass->explorerIcon.empty()) currentClass = currentClass->super;

    QImage icon("assets/icons/" + QString::fromStdString(currentClass->explorerIcon));
    instanceIconCache[type->className] = icon;
    return icon;
}

bool ExplorerModel::moveRows(const QModelIndex &sourceParentIdx, int sourceRow, int count, const QModelIndex &destinationParentIdx, int destinationChild) {
    Instance* sourceParent = sourceParentIdx.isValid() ? static_cast<Instance*>(sourceParentIdx.internalPointer()) : workspace.get();
    Instance* destinationParent = destinationParentIdx.isValid() ? static_cast<Instance*>(destinationParentIdx.internalPointer()) : workspace.get();

    printf("Moved %d from %s\n", count, sourceParent->name.c_str());

    if ((sourceRow + count) >= sourceParent->GetChildren().size()) {
        fprintf(stderr, "Attempt to move rows %d-%d from %s (%s) while it only has %zu children.\n", sourceRow, sourceRow + count, sourceParent->name.c_str(), sourceParent->GetClass()->className.c_str(), sourceParent->GetChildren().size());
        return false;
    }

    for (int i = sourceRow; i < (sourceRow + count); i++) {
        sourceParent->GetChildren()[i]->SetParent(destinationParent->shared_from_this());
    }

    return true;
}

bool ExplorerModel::removeRows(int row, int count, const QModelIndex& parentIdx) {
    Instance* parent = parentIdx.isValid() ? static_cast<Instance*>(parentIdx.internalPointer()) : workspace.get();
    
    for (int i = row; i < (row + count); i++) {
        //parent->GetChildren()[i]->SetParent(nullptr);
    }

    return true;
}

bool ExplorerModel::insertRows(int row, int count, const QModelIndex & parentIdx) {
    //Instance* parent = parentIdx.isValid() ? static_cast<Instance*>(parentIdx.internalPointer()) : workspace.get();
    //beginInsertRows(parentIdx, parent->GetChildren().size(), parent->GetChildren().size() + count);
    //for ()
    //endInsertRows();
    //return true;
    return false;
}

Qt::DropActions ExplorerModel::supportedDragActions() const {
    return Qt::DropAction::MoveAction;
}

Qt::DropActions ExplorerModel::supportedDropActions() const {
    return Qt::DropAction::MoveAction;
}


InstanceRef ExplorerModel::fromIndex(const QModelIndex index) const {
    if (!index.isValid()) return workspace;
    return static_cast<Instance*>(index.internalPointer())->shared_from_this();
}

struct DragDropSlot {
    std::vector<InstanceRef> instances;
};

bool ExplorerModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
//    if (action != Qt::InternalMove) return;
    QByteArray byteData = data->data("application/x-openblocks-instance-pointers");
    uintptr_t slotPtr = byteData.toULongLong();
    DragDropSlot* slot = (DragDropSlot*)slotPtr;
    
    if (!parent.isValid()) {
        delete slot;
        return true;
    }

    InstanceRef parentInst = fromIndex(parent);
    for (InstanceRef instance : slot->instances) {
        instance->SetParent(parentInst);
    }

    delete slot;
    return true;
}

QMimeData* ExplorerModel::mimeData(const QModelIndexList& indexes) const {
    // application/x-openblocks-instance-pointers
    DragDropSlot* slot = new DragDropSlot();

    for (const QModelIndex& index : indexes) {
        slot->instances.push_back(fromIndex(index));
    }

    // uintptr_t ptr = (uintptr_t)&slot;

    QMimeData* mimeData = new QMimeData();
    mimeData->setData("application/x-openblocks-instance-pointers", QByteArray::number((qulonglong)slot));
    return mimeData;
}

QStringList ExplorerModel::mimeTypes() const {
    return QStringList("application/x-openblocks-instance-pointers");
}