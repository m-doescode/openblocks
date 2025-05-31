#include "panes/propertiesview.h"
#include "common.h"
#include "datatypes/base.h"
#include "datatypes/variant.h"
#include "datatypes/primitives.h"
#include "objects/base/member.h"

#include <QColorDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTime>
#include <functional>
#include <qnamespace.h>
#include <qtreewidget.h>

class PropertiesItemDelegate : public QStyledItemDelegate {
    PropertiesView* view;
public:
    PropertiesItemDelegate(PropertiesView* parent) : view(parent), QStyledItemDelegate(parent) {}

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        // https://stackoverflow.com/a/76645757/16255372
        // https://stackoverflow.com/a/70078448/16255372

        int indent = dynamic_cast<PropertiesView*>(parent())->indentation();

        QStyledItemDelegate::initStyleOption(option, index);

        if (!index.parent().isValid()) {
            option->state &= ~QStyle::State_Selected;

            option->backgroundBrush = view->palette().dark();
        }
    };

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        if (index.column() == 0) return nullptr;
    
        if (!index.parent().isValid() || view->currentInstance.expired()) return nullptr;
        std::shared_ptr<Instance> inst = view->currentInstance.lock();

        // If the property is deeper than 1 layer, then it is considered composite
        // Handle specially

        bool isComposite = index.parent().parent().isValid();
        std::string componentName = isComposite ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString() : "";

        std::string propertyName = !isComposite ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString()
            : view->itemFromIndex(index.parent())->data(0, Qt::DisplayRole).toString().toStdString();
        PropertyMeta meta = inst->GetPropertyMeta(propertyName).expect();
        Variant currentValue = inst->GetPropertyValue(propertyName).expect();

        if (isComposite) {
            if (meta.type == &Vector3::TYPE) {
                Vector3 vector = currentValue.get<Vector3>();
                float value = componentName == "X" ? vector.X() : componentName == "Y" ? vector.Y() : componentName == "Z" ? vector.Z() : 0;

                QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
                spinBox->setValue(value);

                return spinBox;
            }

            return nullptr;
        }

        if (meta.type == &FLOAT_TYPE) {
            QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
            spinBox->setValue(currentValue.get<float>());

            if (meta.flags & PROP_UNIT_FLOAT) {
                spinBox->setMinimum(0);
                spinBox->setMaximum(1);
                spinBox->setSingleStep(0.1);
            }

            return spinBox;
        } else if (meta.type == &INT_TYPE) {
            QSpinBox* spinBox = new QSpinBox(parent);
            spinBox->setValue(currentValue.get<int>());

            return spinBox;
        } else if (meta.type == &STRING_TYPE) {
            QLineEdit* lineEdit = new QLineEdit(parent);
            lineEdit->setText(QString::fromStdString(currentValue.get<std::string>()));

            return lineEdit;
        } else if (meta.type == &Color3::TYPE) {
            QColorDialog* colorDialog = new QColorDialog(parent->window());

            Color3 color = currentValue.get<Color3>();
            colorDialog->setCurrentColor(QColor::fromRgbF(color.R(), color.G(), color.B()));

            return colorDialog;
        } else if (meta.type->fromString) {
            QLineEdit* lineEdit = new QLineEdit(parent);
            lineEdit->setText(QString::fromStdString(currentValue.ToString()));

            return lineEdit;
        }

        return nullptr;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        if (index.column() == 0) return;
    
        if (!index.parent().isValid() || view->currentInstance.expired()) return;
        std::shared_ptr<Instance> inst = view->currentInstance.lock();

        bool isComposite = index.parent().parent().isValid();
        std::string componentName = isComposite ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString() : "";

        std::string propertyName = !index.parent().parent().isValid() ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString()
            : view->itemFromIndex(index.parent())->data(0, Qt::DisplayRole).toString().toStdString();
        PropertyMeta meta = inst->GetPropertyMeta(propertyName).expect();
        Variant currentValue = inst->GetPropertyValue(propertyName).expect();
        
        if (isComposite) {
            if (meta.type == &Vector3::TYPE) {
                Vector3 vector = currentValue.get<Vector3>();
                float value = componentName == "X" ? vector.X() : componentName == "Y" ? vector.Y() : componentName == "Z" ? vector.Z() : 0;

                QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);
                spinBox->setValue(value);

                return;
            }
            
            return;
        }

        if (meta.type == &FLOAT_TYPE) {
            QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);

            spinBox->setValue(currentValue.get<float>());
        } else if (meta.type == &INT_TYPE) {
            QSpinBox* spinBox = dynamic_cast<QSpinBox*>(editor);

            spinBox->setValue(currentValue.get<int>());
        } else if (meta.type == &STRING_TYPE) {
            QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);

            lineEdit->setText(QString::fromStdString((std::string)currentValue.get<std::string>()));
        } else if (meta.type == &Color3::TYPE) {
            QColorDialog* colorDialog = dynamic_cast<QColorDialog*>(editor);

            Color3 color = currentValue.get<Color3>();
            colorDialog->setCurrentColor(QColor::fromRgbF(color.R(), color.G(), color.B()));
        } else if (meta.type->fromString) {
            QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);

            lineEdit->setText(QString::fromStdString((std::string)currentValue.ToString()));
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        if (index.column() == 0) return;
    
        if (!index.parent().isValid() || view->currentInstance.expired()) return;
        std::shared_ptr<Instance> inst = view->currentInstance.lock();

        bool isComposite = index.parent().parent().isValid();
        std::string componentName = isComposite ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString() : "";

        std::string propertyName = !index.parent().parent().isValid() ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString()
            : view->itemFromIndex(index.parent())->data(0, Qt::DisplayRole).toString().toStdString();
        PropertyMeta meta = inst->GetPropertyMeta(propertyName).expect();

        if (isComposite) {
            if (meta.type == &Vector3::TYPE) {
                QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);
                float value = spinBox->value();

                Vector3 prev = inst->GetPropertyValue(propertyName).expect().get<Vector3>();
                Vector3 newVector = componentName == "X" ? Vector3(value, prev.Y(), prev.Z())
                : componentName == "Y" ? Vector3(prev.X(), value, prev.Z())
                : componentName == "Z" ? Vector3(prev.X(), prev.Y(), value) : prev;

                inst->SetPropertyValue(propertyName, newVector).expect();
                view->rebuildCompositeProperty(view->itemFromIndex(index.parent()), &Vector3::TYPE, newVector);
                return;
            }

            return;
        }

        if (meta.type == &FLOAT_TYPE) {
            QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);

            inst->SetPropertyValue(propertyName, (float)spinBox->value()).expect();
            model->setData(index, spinBox->value());
        } else if (meta.type == &INT_TYPE) {
            QSpinBox* spinBox = dynamic_cast<QSpinBox*>(editor);

            inst->SetPropertyValue(propertyName, (int)spinBox->value()).expect();
            model->setData(index, spinBox->value());
        } else if (meta.type == &STRING_TYPE) {
            QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);

            inst->SetPropertyValue(propertyName, lineEdit->text().toStdString()).expect();
            model->setData(index, lineEdit->text());
        } else if (meta.type == &Color3::TYPE) {
            QColorDialog* colorDialog = dynamic_cast<QColorDialog*>(editor);

            QColor color = colorDialog->currentColor();
            Color3 color3(color.redF(), color.greenF(), color.blueF());
            inst->SetPropertyValue(propertyName, color3).expect();
            model->setData(index, QString::fromStdString(color3.ToString()), Qt::DisplayRole);
            model->setData(index, color, Qt::DecorationRole);
        } else if (meta.type->fromString) {
            QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);

            std::optional<Variant> parsedResult = meta.type->fromString(lineEdit->text().toStdString());
            if (!parsedResult) return;
            inst->SetPropertyValue(propertyName, parsedResult.value()).expect();
            model->setData(index, QString::fromStdString(parsedResult.value().ToString()));
            view->rebuildCompositeProperty(view->itemFromIndex(index), meta.type, parsedResult.value());
        }
    }
};

PropertiesView::PropertiesView(QWidget* parent):
    QTreeWidget(parent) {
    
    clear();
    setHeaderHidden(true);
    setColumnCount(2);
    setAlternatingRowColors(true);
    setItemDelegate(new PropertiesItemDelegate(this));

    connect(this, &QTreeWidget::itemChanged, this, &PropertiesView::propertyChanged);
    connect(this, &QTreeWidget::itemActivated, this, [&](auto* item, int column) {
        // Prevent editing the first column
        if (column == 0)
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        else if (item->parent())
            item->setFlags(item->flags() | Qt::ItemIsEditable);
    });

    addPropertyUpdateListener(std::bind(&PropertiesView::onPropertyUpdated, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

PropertiesView::~PropertiesView() {
}

QStringList PROPERTY_CATEGORY_NAMES {
    "Appearence",
    "Data",
    "Behavior",
    "Part",
    "Surface",
    "Surface Inputs",
};

QModelIndex PropertiesView::indexAt(const QPoint &point) const {
    return QTreeWidget::indexAt(point + QPoint(indentation(), 0));
}

void PropertiesView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const {
    // https://codebrowser.dev/qt5/qtbase/src/widgets/itemviews/qtreeview.cpp.html#312opt

    // Draw background for headings
    if (!index.parent().isValid())
        painter->fillRect(rect, palette().dark());

    QTreeWidget::drawBranches(painter, rect, index);
}

void PropertiesView::setSelected(std::optional<std::shared_ptr<Instance>> instance) {
    clear();
    currentInstance = {};
    if (!instance) return;
    std::shared_ptr<Instance> inst = instance.value();
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
    std::sort(properties.begin(),properties.end(), [&](auto a, auto b) { return a < b; });

    for (std::string property : properties) {
        PropertyMeta meta = inst->GetPropertyMeta(property).expect();
        Variant currentValue = inst->GetPropertyValue(property).expect();

        if (meta.type == &CFrame::TYPE || meta.flags & PROP_HIDDEN) continue;

        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
        item->setData(0, Qt::DisplayRole, QString::fromStdString(property));
        
        if (meta.type == &BOOL_TYPE) {
            item->setCheckState(1, (bool)currentValue.get<bool>() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        } else if (meta.type == &Color3::TYPE) {
            Color3 color = currentValue.get<Color3>();
            item->setData(1, Qt::DecorationRole, QColor::fromRgbF(color.R(), color.G(), color.B()));
            item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
        } else if (meta.type == &Vector3::TYPE) {
            Vector3 vector = currentValue.get<Vector3>();
            item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
        // } else if (meta.type == &CFrame::TYPE) {
        //     Vector3 vector = currentValue.get<CFrame>().Position();
        //     item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
        } else {
            item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
        }

        if (meta.type != &Color3::TYPE && (!meta.type->fromString || meta.flags & PROP_READONLY)) {
            item->setDisabled(true);
        }

        rebuildCompositeProperty(item, meta.type, currentValue);

        propertyCategories[meta.category]->addChild(item);
        propertyCategories[meta.category]->setExpanded(true);
    }

    // Remove child-less categories
    for (int i = 0; i <= PROPERTY_CATEGORY_MAX; i++) {
        if (i == PROP_CATEGORY_SURFACE_INPUT)
            propertyCategories[(PropertyCategory)i]->setExpanded(false);

        if (propertyCategories[(PropertyCategory)i]->childCount() > 0) continue;
        int idx = indexOfTopLevelItem(propertyCategories[(PropertyCategory)i]);
        delete takeTopLevelItem(idx);
    }

    resizeColumnToContents(0);
}

void PropertiesView::propertyChanged(QTreeWidgetItem *item, int column) {
    // Necessary because otherwise this will catch setCheckState from onPropertyUpdated
    if (ignorePropertyUpdates) return;
    if (!item->parent() || (item->parent() && item->parent()->parent()) || currentInstance.expired()) return;
    std::shared_ptr<Instance> inst = currentInstance.lock();

    std::string propertyName = item->data(0, Qt::DisplayRole).toString().toStdString();
    PropertyMeta meta = inst->GetPropertyMeta(propertyName).expect();

    if (meta.type == &BOOL_TYPE) {
        inst->SetPropertyValue(propertyName, item->checkState(1) == Qt::Checked).expect();
    }
}

void PropertiesView::rebuildCompositeProperty(QTreeWidgetItem *item, const TypeInfo* type, Variant value) {
    if (type == &Vector3::TYPE) {
        // https://forum.qt.io/post/266837
        foreach(auto i, item->takeChildren()) delete i;

        Vector3 vector = value.get<Vector3>();
        item->setData(1, Qt::DisplayRole, QString::fromStdString(value.ToString()));

        QTreeWidgetItem* xItem = new QTreeWidgetItem;
        xItem->setData(0, Qt::DisplayRole, "X");
        xItem->setData(1, Qt::DisplayRole, vector.X());

        QTreeWidgetItem* yItem = new QTreeWidgetItem;
        yItem->setData(0, Qt::DisplayRole, "Y");
        yItem->setData(1, Qt::DisplayRole, vector.Y());

        QTreeWidgetItem* zItem = new QTreeWidgetItem;
        zItem->setData(0, Qt::DisplayRole, "Z");
        zItem->setData(1, Qt::DisplayRole, vector.Z());

        item->addChild(xItem);
        item->addChild(yItem);
        item->addChild(zItem);
    }
}

// static auto lastUpdateTime = std::chrono::steady_clock::now();
void PropertiesView::onPropertyUpdated(std::shared_ptr<Instance> inst, std::string property, Variant newValue) {
    // if (!currentInstance || currentInstance->expired() || instance != currentInstance->lock()) return;
    // if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastUpdateTime).count() < 1000) return;
    // lastUpdateTime = std::chrono::steady_clock::now();

    PropertyMeta meta = inst->GetPropertyMeta(property).expect();
    Variant currentValue = inst->GetPropertyValue(property).expect();
    
    if (meta.type == &CFrame::TYPE) return;

    for (int categoryItemIdx = 0; categoryItemIdx < topLevelItemCount(); categoryItemIdx++) {
        QTreeWidgetItem* categoryItem = topLevelItem(categoryItemIdx);
        for (int itemIdx = 0; itemIdx < categoryItem->childCount(); itemIdx++) {
            QTreeWidgetItem* item = categoryItem->child(itemIdx);

            if (item->data(0, Qt::DisplayRole).toString().toStdString() != property) continue;

            if (meta.type == &BOOL_TYPE) {
                // This is done because otherwise propertyChanged will catch this change erroneously
                ignorePropertyUpdates = true;
                item->setCheckState(1, (bool)currentValue.get<bool>() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
                ignorePropertyUpdates = false;
            } else if (meta.type == &Color3::TYPE) {
                Color3 color = currentValue.get<Color3>();
                item->setData(1, Qt::DecorationRole, QColor::fromRgbF(color.R(), color.G(), color.B()));
                item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
            } else if (meta.type == &Vector3::TYPE) {
                Vector3 vector = currentValue.get<Vector3>();
                item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
            } else {
                item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
            }

            if (meta.type != &Color3::TYPE && (!meta.type->fromString || meta.flags & PROP_READONLY)) {
                item->setDisabled(true);
            }

            rebuildCompositeProperty(item, meta.type, currentValue);

            return;
        }
    }
}