#pragma once

#include <QTreeWidget>
#include "objects/base/instance.h"

class Ui_MainWindow;

class PropertiesView : public QTreeWidget {
    Q_DECLARE_PRIVATE(QTreeView)
protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
public:
    PropertiesView(QWidget* parent = nullptr);
    ~PropertiesView() override;

    void setSelected(std::optional<InstanceRef> instance);
};