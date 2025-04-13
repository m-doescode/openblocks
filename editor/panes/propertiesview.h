#pragma once

#include <QTreeWidget>
#include "datatypes/base.h"
#include "objects/base/instance.h"

class Ui_MainWindow;

class PropertiesItemDelegate;

class PropertiesView : public QTreeWidget {
    Q_DECLARE_PRIVATE(QTreeView)

    std::optional<InstanceRefWeak> currentInstance;
    void propertyChanged(QTreeWidgetItem *item, int column);
    void activateProperty(QTreeWidgetItem *item, int column);
    void rebuildCompositeProperty(QTreeWidgetItem *item, const Data::TypeInfo*, Data::Variant);

    friend PropertiesItemDelegate;
protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    QModelIndex indexAt(const QPoint &point) const override;
public:
    PropertiesView(QWidget* parent = nullptr);
    ~PropertiesView() override;

    void setSelected(std::optional<InstanceRef> instance);
};