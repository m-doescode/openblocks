#include "panes/propertiesview.h"
#include "common.h"
#include "datatypes/base.h"

#include <QColorDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTime>
#include <chrono>
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
    
        if (!index.parent().isValid() || !view->currentInstance || view->currentInstance->expired()) return nullptr;
        InstanceRef inst = view->currentInstance->lock();

        // If the property is deeper than 1 layer, then it is considered composite
        // Handle specially

        bool isComposite = index.parent().parent().isValid();
        std::string componentName = isComposite ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString() : "";

        std::string propertyName = !isComposite ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString()
            : view->itemFromIndex(index.parent())->data(0, Qt::DisplayRole).toString().toStdString();
        PropertyMeta meta = inst->GetPropertyMeta(propertyName).expect();
        Data::Variant currentValue = inst->GetPropertyValue(propertyName).expect();

        if (isComposite) {
            if (meta.type == &Data::Vector3::TYPE) {
                Data::Vector3 vector = currentValue.get<Data::Vector3>();
                float value = componentName == "X" ? vector.X() : componentName == "Y" ? vector.Y() : componentName == "Z" ? vector.Z() : 0;

                QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
                spinBox->setValue(value);

                return spinBox;
            }

            return nullptr;
        }

        if (meta.type == &Data::Float::TYPE) {
            QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
            spinBox->setValue(currentValue.get<Data::Float>());

            if (meta.flags & PROP_UNIT_FLOAT) {
                spinBox->setMinimum(0);
                spinBox->setMaximum(1);
                spinBox->setSingleStep(0.1);
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
        } else if (meta.type == &Data::Color3::TYPE) {
            QColorDialog* colorDialog = new QColorDialog(parent->window());

            Data::Color3 color = currentValue.get<Data::Color3>();
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
    
        if (!index.parent().isValid() || !view->currentInstance || view->currentInstance->expired()) return;
        InstanceRef inst = view->currentInstance->lock();

        bool isComposite = index.parent().parent().isValid();
        std::string componentName = isComposite ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString() : "";

        std::string propertyName = !index.parent().parent().isValid() ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString()
            : view->itemFromIndex(index.parent())->data(0, Qt::DisplayRole).toString().toStdString();
        PropertyMeta meta = inst->GetPropertyMeta(propertyName).expect();
        Data::Variant currentValue = inst->GetPropertyValue(propertyName).expect();
        
        if (isComposite) {
            if (meta.type == &Data::Vector3::TYPE) {
                Data::Vector3 vector = currentValue.get<Data::Vector3>();
                float value = componentName == "X" ? vector.X() : componentName == "Y" ? vector.Y() : componentName == "Z" ? vector.Z() : 0;

                QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);
                spinBox->setValue(value);

                return;
            }
            
            return;
        }

        if (meta.type == &Data::Float::TYPE) {
            QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);

            spinBox->setValue(currentValue.get<Data::Float>());
        } else if (meta.type == &Data::Int::TYPE) {
            QSpinBox* spinBox = dynamic_cast<QSpinBox*>(editor);

            spinBox->setValue(currentValue.get<Data::Int>());
        } else if (meta.type == &Data::String::TYPE) {
            QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);

            lineEdit->setText(QString::fromStdString((std::string)currentValue.get<Data::String>()));
        } else if (meta.type == &Data::Color3::TYPE) {
            QColorDialog* colorDialog = dynamic_cast<QColorDialog*>(editor);

            Data::Color3 color = currentValue.get<Data::Color3>();
            colorDialog->setCurrentColor(QColor::fromRgbF(color.R(), color.G(), color.B()));
        } else if (meta.type->fromString) {
            QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);

            lineEdit->setText(QString::fromStdString((std::string)currentValue.ToString()));
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        if (index.column() == 0) return;
    
        if (!index.parent().isValid() || !view->currentInstance || view->currentInstance->expired()) return;
        InstanceRef inst = view->currentInstance->lock();

        bool isComposite = index.parent().parent().isValid();
        std::string componentName = isComposite ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString() : "";

        std::string propertyName = !index.parent().parent().isValid() ? view->itemFromIndex(index)->data(0, Qt::DisplayRole).toString().toStdString()
            : view->itemFromIndex(index.parent())->data(0, Qt::DisplayRole).toString().toStdString();
        PropertyMeta meta = inst->GetPropertyMeta(propertyName).expect();

        if (isComposite) {
            if (meta.type == &Data::Vector3::TYPE) {
                QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);
                float value = spinBox->value();

                Data::Vector3 prev = inst->GetPropertyValue(propertyName).expect().get<Data::Vector3>();
                Data::Vector3 newVector = componentName == "X" ? Data::Vector3(value, prev.Y(), prev.Z())
                : componentName == "Y" ? Data::Vector3(prev.X(), value, prev.Z())
                : componentName == "Z" ? Data::Vector3(prev.X(), prev.Y(), value) : prev;

                inst->SetPropertyValue(propertyName, newVector).expect();
                view->rebuildCompositeProperty(view->itemFromIndex(index.parent()), &Data::Vector3::TYPE, newVector);
                return;
            }

            return;
        }

        if (meta.type == &Data::Float::TYPE) {
            QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);

            inst->SetPropertyValue(propertyName, Data::Float((float)spinBox->value())).expect();
            model->setData(index, spinBox->value());
        } else if (meta.type == &Data::Int::TYPE) {
            QSpinBox* spinBox = dynamic_cast<QSpinBox*>(editor);

            inst->SetPropertyValue(propertyName, Data::Int((float)spinBox->value())).expect();
            model->setData(index, spinBox->value());
        } else if (meta.type == &Data::String::TYPE) {
            QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);

            inst->SetPropertyValue(propertyName, Data::String(lineEdit->text().toStdString())).expect();
            model->setData(index, lineEdit->text());
        } else if (meta.type == &Data::Color3::TYPE) {
            QColorDialog* colorDialog = dynamic_cast<QColorDialog*>(editor);

            QColor color = colorDialog->currentColor();
            Data::Color3 color3(color.redF(), color.greenF(), color.blueF());
            inst->SetPropertyValue(propertyName, color3).expect();
            model->setData(index, QString::fromStdString(color3.ToString()), Qt::DisplayRole);
            model->setData(index, color, Qt::DecorationRole);
        } else if (meta.type->fromString) {
            QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);

            std::optional<Data::Variant> parsedResult = meta.type->fromString(lineEdit->text().toStdString());
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
    "Surface"
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
        PropertyMeta meta = inst->GetPropertyMeta(property).expect();
        Data::Variant currentValue = inst->GetPropertyValue(property).expect();

        if (meta.type == &Data::CFrame::TYPE) continue;

        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
        item->setData(0, Qt::DisplayRole, QString::fromStdString(property));
        
        if (meta.type == &Data::Bool::TYPE) {
            item->setCheckState(1, (bool)currentValue.get<Data::Bool>() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        } else if (meta.type == &Data::Color3::TYPE) {
            Data::Color3 color = currentValue.get<Data::Color3>();
            item->setData(1, Qt::DecorationRole, QColor::fromRgbF(color.R(), color.G(), color.B()));
            item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
        } else if (meta.type == &Data::Vector3::TYPE) {
            Data::Vector3 vector = currentValue.get<Data::Vector3>();
            item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
        } else {
            item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
        }

        if (meta.type != &Data::Color3::TYPE && (!meta.type->fromString || meta.flags & PROP_READONLY)) {
            item->setDisabled(true);
        }

        rebuildCompositeProperty(item, meta.type, currentValue);

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
    if (!item->parent() || (item->parent() && item->parent()->parent()) || !currentInstance || currentInstance->expired()) return;
    InstanceRef inst = currentInstance->lock();

    std::string propertyName = item->data(0, Qt::DisplayRole).toString().toStdString();
    PropertyMeta meta = inst->GetPropertyMeta(propertyName).expect();

    if (meta.type == &Data::Bool::TYPE) {
        inst->SetPropertyValue(propertyName, Data::Bool(item->checkState(1))).expect();
    }
}

void PropertiesView::rebuildCompositeProperty(QTreeWidgetItem *item, const Data::TypeInfo* type, Data::Variant value) {
    if (type == &Data::Vector3::TYPE) {
        // https://forum.qt.io/post/266837
        foreach(auto i, item->takeChildren()) delete i;

        Data::Vector3 vector = value.get<Data::Vector3>();
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
void PropertiesView::onPropertyUpdated(InstanceRef inst, std::string property, Data::Variant newValue) {
    // if (!currentInstance || currentInstance->expired() || instance != currentInstance->lock()) return;
    // if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastUpdateTime).count() < 1000) return;
    // lastUpdateTime = std::chrono::steady_clock::now();

    PropertyMeta meta = inst->GetPropertyMeta(property).expect();
    Data::Variant currentValue = inst->GetPropertyValue(property).expect();
    
    if (meta.type == &Data::CFrame::TYPE) return;

    for (int categoryItemIdx = 0; categoryItemIdx < topLevelItemCount(); categoryItemIdx++) {
        QTreeWidgetItem* categoryItem = topLevelItem(categoryItemIdx);
        for (int itemIdx = 0; itemIdx < categoryItem->childCount(); itemIdx++) {
            QTreeWidgetItem* item = categoryItem->child(itemIdx);

            if (item->data(0, Qt::DisplayRole).toString().toStdString() != property) continue;

            if (meta.type == &Data::Bool::TYPE) {
                item->setCheckState(1, (bool)currentValue.get<Data::Bool>() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
            } else if (meta.type == &Data::Color3::TYPE) {
                Data::Color3 color = currentValue.get<Data::Color3>();
                item->setData(1, Qt::DecorationRole, QColor::fromRgbF(color.R(), color.G(), color.B()));
                item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
            } else if (meta.type == &Data::Vector3::TYPE) {
                Data::Vector3 vector = currentValue.get<Data::Vector3>();
                item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
            } else {
                item->setData(1, Qt::DisplayRole, QString::fromStdString(currentValue.ToString()));
            }

            if (meta.type != &Data::Color3::TYPE && (!meta.type->fromString || meta.flags & PROP_READONLY)) {
                item->setDisabled(true);
            }

            rebuildCompositeProperty(item, meta.type, currentValue);

            return;
        }
    }
}