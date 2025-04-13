#pragma once

#include <QTreeWidget>
#include "objects/base/instance.h"

class Ui_MainWindow;

class CustomItemDelegate;

class PropertiesView : public QTreeWidget {
    Q_DECLARE_PRIVATE(QTreeView)

    std::optional<InstanceRefWeak> currentInstance;
    void propertyChanged(QTreeWidgetItem *item, int column);
    void activateProperty(QTreeWidgetItem *item, int column);

    friend CustomItemDelegate;
protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    QModelIndex indexAt(const QPoint &point) const override;
public:
    PropertiesView(QWidget* parent = nullptr);
    ~PropertiesView() override;

    void setSelected(std::optional<InstanceRef> instance);
};