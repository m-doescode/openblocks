#pragma once

#include <QTreeWidget>
#include "datatypes/base.h"
#include "objects/base/instance.h"
#include "undohistory.h"

class Ui_MainWindow;
class PropertiesItemDelegate;
namespace Data { class Variant; };

class PropertiesView : public QTreeWidget {
    Q_DECLARE_PRIVATE(QTreeView)

    bool ignorePropertyUpdates = false;
    std::weak_ptr<Instance> currentInstance;
    void propertyChanged(QTreeWidgetItem *item, int column);
    void activateProperty(QTreeWidgetItem *item, int column);
    void rebuildCompositeProperty(QTreeWidgetItem *item, const TypeDesc*, Variant);
    void onPropertyUpdated(std::shared_ptr<Instance> instance, std::string property, Variant newValue);

    UndoHistory* undoManager;

    friend PropertiesItemDelegate;
protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    QModelIndex indexAt(const QPoint &point) const override;
public:
    PropertiesView(QWidget* parent = nullptr);
    ~PropertiesView() override;

    void init();

    void setSelected(std::optional<std::shared_ptr<Instance>> instance);
};