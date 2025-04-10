#pragma once

#include "objects/base/instance.h"
#include "qabstractitemmodel.h"
#include "qnamespace.h"
#include <QOpenGLWidget>
#include <QWidget>

class PropertiesModel : public QAbstractItemModel {
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(PropertiesModel)

    explicit PropertiesModel(InstanceRef selectedItem, QWidget *parent = nullptr);
    ~PropertiesModel() override;

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

private:
    InstanceRef selectedItem;
    std::vector<std::string> propertiesList;
};