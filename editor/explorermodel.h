#ifndef EXPLORERMODEL_H
#define EXPLORERMODEL_H

#include "objects/base/instance.h"
#include "objects/part.h"
#include "qabstractitemmodel.h"
#include "qevent.h"
#include <QOpenGLWidget>
#include <QWidget>
#include <memory>

class ExplorerModel : public QAbstractItemModel {
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(ExplorerModel)

    explicit ExplorerModel(InstanceRef dataRoot, QWidget *parent = nullptr);
    ~ExplorerModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;

private:
    InstanceRef rootItem;
    QModelIndex toIndex(InstanceRef item);
};

#endif // EXPLORERMODEL_H
