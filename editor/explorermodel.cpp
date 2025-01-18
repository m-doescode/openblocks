#include "explorermodel.h"
#include "objects/base/instance.h"
#include "qcontainerfwd.h"
#include "qobject.h"
#include "qwidget.h"
#include "common.h"
#include <algorithm>
#include <optional>

// https://doc.qt.io/qt-6/qtwidgets-itemviews-simpletreemodel-example.html#testing-the-model

ExplorerModel::ExplorerModel(InstanceRef dataRoot, QWidget *parent)
    : QAbstractItemModel(parent)
    , rootItem(dataRoot) {
    // TODO: Don't use lambdas and handlers like that
    hierarchyPreUpdateHandler = [&](InstanceRef object, std::optional<InstanceRef> oldParent, std::optional<InstanceRef> newParent) {
        if (oldParent.has_value()) {
            auto children = oldParent.value()->GetChildren();
            size_t idx = std::find(children.begin(), children.end(), object) - children.end();
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
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    const Instance *item = static_cast<const Instance*>(index.internalPointer());
    return QString::fromStdString(item->name);
}

QVariant ExplorerModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    return QString("Idk lol \u00AF\u005C\u005F\u0028\u30C4\u0029\u005F\u002F\u00AF");
}

Qt::ItemFlags ExplorerModel::flags(const QModelIndex &index) const
{
    return index.isValid()
        ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}