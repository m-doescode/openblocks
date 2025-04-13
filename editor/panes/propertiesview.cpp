#include "propertiesview.h"
#include "datatypes/base.h"
#include "datatypes/meta.h"
#include "objects/base/member.h"
#include <map>
#include <qabstractitemdelegate.h>
#include <qbrush.h>
#include <qlineedit.h>
#include <qnamespace.h>
#include <qpalette.h>
#include <qspinbox.h>
#include <qstyle.h>
#include <qstyleditemdelegate.h>
#include <qstyleoption.h>
#include <qtreewidget.h>
#include <QDebug>
#include <QStyledItemDelegate>
#include <private/qtreeview_p.h>
#include <QDoubleSpinBox>

class CustomItemDelegate : public QStyledItemDelegate {
    PropertiesView* view;
public:
    CustomItemDelegate(PropertiesView* parent) : view(parent), QStyledItemDelegate(parent) {}

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        // https://stackoverflow.com/a/76645757/16255372
        // https://stackoverflow.com/a/70078448/16255372

        int indent = dynamic_cast<PropertiesView*>(parent())->indentation();

        QStyledItemDelegate::initStyleOption(option, index);

        if (index.parent().isValid()) {
            option->rect.adjust(-indent, 0, -indent, 0);
        } else {
            option->state &= ~QStyle::State_Selected;

            option->backgroundBrush = view->palette().dark();
        }
    };

    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        if (index.column() == 0) return nullptr;
    
        if (!index.parent().isValid() || !view->currentInstance || view->currentInstance->expired()) return nullptr;
        InstanceRef inst = view->currentInstance->lock();
    
        std::string propertyName = view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString();
        PropertyMeta meta = inst->GetPropertyMeta(propertyName).value();
        Data::Variant currentValue = inst->GetPropertyValue(propertyName).value();

        if (meta.type == &Data::Float::TYPE) {
            QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
            spinBox->setValue(currentValue.get<Data::Float>());

            if (meta.flags & PROP_UNIT_FLOAT) {
                spinBox->setMinimum(0);
                spinBox->setMaximum(1);
            }

            return spinBox;
        } else if (meta.type == &Data::Int::TYPE) {
            QSpinBox* spinBox = new QSpinBox(parent);
            spinBox->setValue(currentValue.get<Data::Int>());

            return spinBox;
        } else if (meta.type == &Data::String::TYPE) {
            QLineEdit* lineEdit = new QLineEdit(parent);
            lineEdit->setText(QString::fromStdString(currentValue.get<Data::String>()));

            return lineEdit;
        }

        return nullptr;
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const override {
        editor->setGeometry(option.rect.adjusted(-view->indentation(), 0, -view->indentation(), 0));
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        if (index.column() == 0) return;
    
        if (!index.parent().isValid() || !view->currentInstance || view->currentInstance->expired()) return;
        InstanceRef inst = view->currentInstance->lock();

        std::string propertyName = view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString();
        PropertyMeta meta = inst->GetPropertyMeta(propertyName).value();

        if (meta.type == &Data::Float::TYPE) {
            QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);

            inst->SetPropertyValue(propertyName, Data::Float((float)spinBox->value()));
        } else if (meta.type == &Data::Int::TYPE) {
            QSpinBox* spinBox = dynamic_cast<QSpinBox*>(editor);

            inst->SetPropertyValue(propertyName, Data::Int((float)spinBox->value()));
        } else if (meta.type == &Data::String::TYPE) {
            QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);

            inst->SetPropertyValue(propertyName, Data::String(lineEdit->text().toStdString()));
        }
    }
};

PropertiesView::PropertiesView(QWidget* parent):
    QTreeWidget(parent) {
    
    clear();
    setHeaderHidden(true);
    setColumnCount(2);
    setAlternatingRowColors(true);
    setItemDelegate(new CustomItemDelegate(this));

    connect(this, &QTreeWidget::itemChanged, this, &PropertiesView::propertyChanged);
    connect(this, &QTreeWidget::itemActivated, this, [&](auto* item, int column) {
        // Prevent editing the first column
        if (column == 0)
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        else if (item->parent())
            item->setFlags(item->flags() | Qt::ItemIsEditable);
    });
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

QModelIndex PropertiesView::indexAt(const QPoint &point) const {
    return QTreeWidget::indexAt(point + QPoint(indentation(), 0));
}

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
    currentInstance = inst;

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
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
        item->setData(0, Qt::DisplayRole, QString::fromStdString(property));
        
        if (meta.type == &Data::Bool::TYPE) {
            item->setCheckState(1, (bool)currentValue.get<Data::Bool>() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        } else {
            item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
        }

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

void PropertiesView::propertyChanged(QTreeWidgetItem *item, int column) {
    if (!item->parent() || !currentInstance || currentInstance->expired()) return;
    InstanceRef inst = currentInstance->lock();

    // std::string propertyName = item->data(0, Qt::DisplayRole).toString().toStdString();
    // PropertyMeta meta = inst->GetPropertyMeta(propertyName).value();

    // if (meta.type == &Data::String::TYPE) {
    //     inst->SetPropertyValue(propertyName, Data::String(item->data(1, Qt::EditRole).toString().toStdString()));
    // } else if (meta.type == &Data::Bool::TYPE) {
    //     inst->SetPropertyValue(propertyName, Data::Bool(item->checkState(1)));
    // } else {
    //     inst->SetPropertyValue(propertyName, meta.type->fromString(item->data(1, Qt::EditRole).toString().toStdString()));
    // }

    // update(indexFromItem(item, column));
}