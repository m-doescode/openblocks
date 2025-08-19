#include "explorermodel.h"
#include "common.h"
#include "mainwindow.h"
#include "objects/base/instance.h"
#include "objects/base/member.h"
#include "undohistory.h"
#include <qicon.h>
#include <qmimedata.h>
#include <QWidget>

#ifdef _NDEBUG
#define NDEBUG
#endif

#define M_mainWindow dynamic_cast<MainWindow*>(dynamic_cast<QWidget*>(dynamic_cast<QObject*>(this)->parent())->window())

// https://doc.qt.io/qt-6/qtwidgets-itemviews-simpletreemodel-example.html#testing-the-model

std::map<std::string, QIcon> instanceIconCache;

ExplorerModel::ExplorerModel(std::shared_ptr<Instance> dataRoot, QWidget *parent)
    : QAbstractItemModel(parent)
    , rootItem(dataRoot) {
    // TODO: Don't use lambdas and handlers like that
    hierarchyPreUpdateHandler = [&](std::shared_ptr<Instance> object, nullable std::shared_ptr<Instance> oldParent, nullable std::shared_ptr<Instance> newParent) {
        if (oldParent) {
            auto children = oldParent->GetChildren();
            size_t idx = std::find(children.begin(), children.end(), object) - children.begin();
            beginRemoveRows(toIndex(oldParent), idx, idx);
        }

        if (newParent) {
            size_t size = newParent->GetChildren().size();
            beginInsertRows(toIndex(newParent), size, size);
        } else {
            // TODO:
        }
    };

    hierarchyPostUpdateHandler = [&](std::shared_ptr<Instance> object, nullable std::shared_ptr<Instance> oldParent, nullable std::shared_ptr<Instance> newParent) {
        if (newParent) endInsertRows();
        if (oldParent) endRemoveRows();
    };
}

ExplorerModel::~ExplorerModel() = default;

QModelIndex ExplorerModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return {};

    Instance* parentItem = parent.isValid()
        ? static_cast<Instance*>(parent.internalPointer())
        : rootItem.get();

#ifdef NDEBUG
    if (parentItem->GetChildren().size() >= (size_t)row && !(parentItem->GetChildren()[row]->GetClass()->flags & INSTANCE_HIDDEN))
        return createIndex(row, column, parentItem->GetChildren()[row].get());
#else
    // Don't hide in debug builds
    if (parentItem->GetChildren().size() >= (size_t)row)
        return createIndex(row, column, parentItem->GetChildren()[row].get());
#endif
    return {};
}

QModelIndex ExplorerModel::toIndex(std::shared_ptr<Instance> item) {
    if (item == rootItem || !item->GetParent())
        return {};

    std::shared_ptr<Instance> parentItem = item->GetParent();
    // Check above ensures this item is not root, so value() must be valid
    for (size_t i = 0; i < parentItem->GetChildren().size(); i++)
        if (parentItem->GetChildren()[i] == item)
            return createIndex(i, 0, item.get());
    return QModelIndex{};
}

QModelIndex ExplorerModel::ObjectToIndex(std::shared_ptr<Instance> item) {
    return toIndex(item);
}

QModelIndex ExplorerModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return {};

    Instance* childItem = static_cast<Instance*>(index.internalPointer());
    // NORISK: The parent must exist if the child was obtained from it during this frame
    std::shared_ptr<Instance> parentItem = childItem->GetParent();

    if (parentItem == rootItem)
        return {};

    // Check above ensures this item is not root, so value() must be valid
    std::shared_ptr<Instance> parentParent = parentItem->GetParent();
    for (size_t i = 0; i < parentParent->GetChildren().size(); i++)
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

#ifdef NDEBUG
    // Trim trailing hidden items as they make the branches look weird
    int count = parentItem->GetChildren().size();
    while (count > 0 && parentItem->GetChildren()[count-1]->GetClass()->flags & INSTANCE_HIDDEN) count--;
    return count;
#else
    // Don't hide in debug builds
    return parentItem->GetChildren().size();
#endif
}

int ExplorerModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

QVariant ExplorerModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return {};

    Instance *item = static_cast<Instance*>(index.internalPointer());

    switch (role) {
        case Qt::EditRole:
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
        ? QAbstractItemModel::flags(index) | Qt::ItemIsEditable | (!fromIndex(index)->IsParentLocked() ? Qt::ItemIsDragEnabled : Qt::NoItemFlags) | Qt::ItemIsDropEnabled
        : Qt::NoItemFlags | Qt::ItemIsDropEnabled;
}

QIcon ExplorerModel::iconOf(const InstanceType* type) const {
    if (instanceIconCache.count(type->className)) return instanceIconCache[type->className];

    const InstanceType* currentClass = type;
    while (currentClass->explorerIcon.empty()) currentClass = currentClass->super;

    QIcon icon("assets/icons/" + QString::fromStdString(currentClass->explorerIcon));
    instanceIconCache[type->className] = icon;
    return icon;
}

bool ExplorerModel::moveRows(const QModelIndex &sourceParentIdx, int sourceRow, int count, const QModelIndex &destinationParentIdx, int destinationChild) {
    Instance* sourceParent = sourceParentIdx.isValid() ? static_cast<Instance*>(sourceParentIdx.internalPointer()) : rootItem.get();
    Instance* destinationParent = destinationParentIdx.isValid() ? static_cast<Instance*>(destinationParentIdx.internalPointer()) : rootItem.get();

    Logger::infof("Moved %d from %s", count, sourceParent->name.c_str());

    if (size_t(sourceRow + count) >= sourceParent->GetChildren().size()) {
        Logger::fatalErrorf("Attempt to move rows %d-%d from %s (%s) while it only has %zu children.", sourceRow, sourceRow + count, sourceParent->name.c_str(), sourceParent->GetClass()->className.c_str(), sourceParent->GetChildren().size());
        return false;
    }

    for (int i = sourceRow; i < (sourceRow + count); i++) {
        sourceParent->GetChildren()[i]->SetParent(destinationParent->shared_from_this());
    }

    return true;
}

bool ExplorerModel::removeRows(int row, int count, const QModelIndex& parentIdx) {
    // Instance* parent = parentIdx.isValid() ? static_cast<Instance*>(parentIdx.internalPointer()) : rootItem.get();
    
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


std::shared_ptr<Instance> ExplorerModel::fromIndex(const QModelIndex index) const {
    if (!index.isValid()) return rootItem;
    return static_cast<Instance*>(index.internalPointer())->shared_from_this();
}

struct DragDropSlot {
    std::vector<std::shared_ptr<Instance>> instances;
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

    UndoState historyState;
    std::shared_ptr<Instance> parentInst = fromIndex(parent);
    for (std::shared_ptr<Instance> instance : slot->instances) {
        historyState.push_back(UndoStateInstanceReparented { instance, instance->GetParent(), parentInst });
        instance->SetParent(parentInst);
    }

    M_mainWindow->undoManager.PushState(historyState);

    delete slot;
    return true;
}

void ExplorerModel::updateRoot(std::shared_ptr<Instance> newRoot) {
    beginResetModel();
    rootItem = newRoot;
    endResetModel();
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