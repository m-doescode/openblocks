#include "propertiesmodel.h"
#include "datatypes/base.h"
#include "datatypes/cframe.h"
#include "objects/base/member.h"
#include "qnamespace.h"

PropertiesModel::PropertiesModel(InstanceRef selectedItem, QWidget *parent)
    : QAbstractItemModel(parent)
    , selectedItem(selectedItem) {
    this->propertiesList.reserve(selectedItem->GetProperties().size());
    for (std::string name : selectedItem->GetProperties()) {
        PropertyMeta meta = selectedItem->GetPropertyMeta(name).value();
        // Don't show CFrames in properties
        if (meta.type == &Data::CFrame::TYPE) continue;

        this->propertiesList.push_back(name);
    }
}

PropertiesModel::~PropertiesModel() = default;


QVariant PropertiesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return {};

    std::string propertyName = propertiesList[index.row()];
    PropertyMeta meta = selectedItem->GetPropertyMeta(propertyName).value();

    switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            if (index.column() == 0)
                return QString::fromStdString(propertyName);
            else if (index.column() == 1 && meta.type != &Data::Bool::TYPE) {
                return QString::fromStdString(selectedItem->GetPropertyValue(propertyName).value().ToString());
            }
            return {};
        case Qt::CheckStateRole:
            if (index.column() == 0) return {};
            else if (index.column() == 1 && meta.type == &Data::Bool::TYPE)
                return selectedItem->GetPropertyValue(propertyName)->get<Data::Bool>() ? Qt::Checked : Qt::Unchecked;
            return {};
        // case Qt::DecorationRole:
        //     return iconOf(item->GetClass());
    }

    return {};
}

bool PropertiesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.column() != 1) return false;

    std::string propertyName = propertiesList[index.row()];
    PropertyMeta meta = selectedItem->GetPropertyMeta(propertyName).value();

    switch (role) {
    case Qt::EditRole:
        if (!meta.type->fromString)
            return false;

        selectedItem->SetPropertyValue(propertyName, meta.type->fromString(value.toString().toStdString()));
        return true;
    case Qt::CheckStateRole:
        if (meta.type != &Data::Bool::TYPE)
            return false;

        selectedItem->SetPropertyValue(propertyName, Data::Bool(value.toBool()));
        return true;
    }

    return false;
}

Qt::ItemFlags PropertiesModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == 0)
        return Qt::ItemIsEnabled;

    std::string propertyName = propertiesList[index.row()];
    PropertyMeta meta = selectedItem->GetPropertyMeta(propertyName).value();

    if (index.column() == 1) {
        if (meta.type == &Data::Bool::TYPE)
            return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
        else
            return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

    return Qt::NoItemFlags;
};

QVariant PropertiesModel::headerData(int section, Qt::Orientation orientation,
                    int role) const {
    return QString("");
}

QModelIndex PropertiesModel::index(int row, int column,
                    const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return {};

    return createIndex(row, column);
}

QModelIndex PropertiesModel::parent(const QModelIndex &index) const {
    return {};
}

int PropertiesModel::rowCount(const QModelIndex &parent) const {
    return !parent.isValid() ? this->propertiesList.size() : 0;
}

int PropertiesModel::columnCount(const QModelIndex &parent) const {
    return 2;
}