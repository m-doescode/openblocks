#include "propertiesview.h"
#include "datatypes/meta.h"
#include "objects/base/member.h"
#include "propertiesmodel.h"
#include "qaction.h"
#include <array>
#include <map>
#include <qabstractitemdelegate.h>
#include <qbrush.h>
#include <qnamespace.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qstyleditemdelegate.h>
#include <qstyleoption.h>
#include <qtreewidget.h>
#include <QDebug>
#include <QStyledItemDelegate>
#include <private/qtreeview_p.h>

class CustomItemDelegate : public QStyledItemDelegate {
public:
    CustomItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        // https://stackoverflow.com/a/76645757/16255372
        // https://stackoverflow.com/a/70078448/16255372

        int indent = dynamic_cast<PropertiesView*>(parent())->indentation();

        QStyledItemDelegate::initStyleOption(option, index);

        if (index.parent().isValid()) {
            option->rect.adjust(-indent, 0, -indent, 0);
        } else {
            option->state &= ~QStyle::State_Selected;

            QWidget* parentWidget = dynamic_cast<QWidget*>(parent());
            option->backgroundBrush = parentWidget->palette().dark();
        }
    };
};

PropertiesView::PropertiesView(QWidget* parent):
    QTreeWidget(parent) {
    
    clear();
    setHeaderHidden(true);
    setColumnCount(2);
    setAlternatingRowColors(true);
    setItemDelegate(new CustomItemDelegate(this));
}

PropertiesView::~PropertiesView() {
}

QStringList PROPERTY_CATEGORY_NAMES {
    "Data",
    "Appearence",
    "Behavior",
    "Part",
    "Surface"
};

void PropertiesView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const {
    // https://codebrowser.dev/qt5/qtbase/src/widgets/itemviews/qtreeview.cpp.html#312opt
    Q_D(const QTreeView);
    QStyleOptionViewItem opt = viewOptions();

    const QTreeViewItem& viewItem = d->viewItems.at(d->current);
    
    // Taken from source code (above)
    bool hoverRow = selectionBehavior() == QAbstractItemView::SelectRows
        && opt.showDecorationSelected
        && index.parent() == d->hover.parent()
        && index.row() == d->hover.row();

    // Un-indent branch
    opt.rect = rect;
    if (index.parent().isValid())
        opt.rect.adjust(0, 0, -indentation(), 0);
    opt.state |= QStyle::State_Item;
    if (viewItem.hasChildren)
        opt.state |= QStyle::State_Children;
    if (viewItem.expanded)
        opt.state |= QStyle::State_Open;
    if (viewItem.hasMoreSiblings || viewItem.parentItem > -1 && d->viewItems.at(viewItem.parentItem).hasMoreSiblings)
        opt.state |= QStyle::State_Sibling;

    opt.state.setFlag(QStyle::State_MouseOver, hoverRow || d->current == d->hoverBranch);

    // Draw background for headings
    if (!index.parent().isValid())
        painter->fillRect(opt.rect, palette().dark());

    style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
}

void PropertiesView::setSelected(std::optional<InstanceRef> instance) {
    clear();
    if (!instance) return;
    InstanceRef inst = instance.value();

    std::map<PropertyCategory, QTreeWidgetItem*> propertyCategories;

    for (int i = 0; i <= PROPERTY_CATEGORY_MAX; i++) {
        QTreeWidgetItem* item = new QTreeWidgetItem;

        QBrush brush;
        brush.setColor(QPalette::Midlight);
        brush.setStyle(Qt::SolidPattern);

        item->setData(0, Qt::DisplayRole, PROPERTY_CATEGORY_NAMES[i]);
        item->setFirstColumnSpanned(true);

        propertyCategories[(PropertyCategory)i] = item;
        addTopLevelItem(item);
    }

    std::vector<std::string> properties = inst->GetProperties();

    for (std::string property : properties) {
        PropertyMeta meta = inst->GetPropertyMeta(property).value();
        Data::Variant currentValue = inst->GetPropertyValue(property).value();

        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setData(0, Qt::DisplayRole, QString::fromStdString(property));
        item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));

        propertyCategories[meta.category]->addChild(item);
        propertyCategories[meta.category]->setExpanded(true);
    }

    // Remove child-less categories
    for (int i = 0; i <= PROPERTY_CATEGORY_MAX; i++) {
        if (propertyCategories[(PropertyCategory)i]->childCount() > 0) continue;
        int idx = indexOfTopLevelItem(propertyCategories[(PropertyCategory)i]);
        delete takeTopLevelItem(idx);
    }

    resizeColumnToContents(0);
}