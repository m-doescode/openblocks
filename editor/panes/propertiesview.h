#pragma once

#include <QTreeWidget>
#include "datatypes/base.h"
#include "objects/base/instance.h"

class Ui_MainWindow;
class PropertiesItemDelegate;
namespace Data { class Variant; };

class PropertiesView : public QTreeWidget {
    Q_DECLARE_PRIVATE(QTreeView)

    InstanceRefWeak currentInstance;
    void propertyChanged(QTreeWidgetItem *item, int column);
    void activateProperty(QTreeWidgetItem *item, int column);
    void rebuildCompositeProperty(QTreeWidgetItem *item, const Data::TypeInfo*, Data::Variant);
    void onPropertyUpdated(InstanceRef instance, std::string property, Data::Variant newValue);

    friend PropertiesItemDelegate;
protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    QModelIndex indexAt(const QPoint &point) const override;
public:
    PropertiesView(QWidget* parent = nullptr);
    ~PropertiesView() override;

    void setSelected(std::optional<InstanceRef> instance);
};