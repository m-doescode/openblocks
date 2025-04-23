#pragma once

#include "objects/base/instance.h"
#include "qabstractitemmodel.h"

class ExplorerModel : public QAbstractItemModel {
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(ExplorerModel)

    explicit ExplorerModel(InstanceRef dataRoot, QWidget *parent = nullptr);
    ~ExplorerModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    InstanceRef fromIndex(const QModelIndex index) const;
    QModelIndex ObjectToIndex(InstanceRef item);
    
    QIcon iconOf(const InstanceType* type) const;

    void updateRoot(InstanceRef newRoot);
private:
    InstanceRef rootItem;
    QModelIndex toIndex(InstanceRef item);
};

// #endif