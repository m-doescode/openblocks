#include "propertiesmodel.h"
#include "qnamespace.h"

PropertiesModel::PropertiesModel(InstanceRef selectedItem, QWidget *parent)
    : QAbstractItemModel(parent)
    , selectedItem(selectedItem) {
    this->propertiesList = selectedItem->GetProperties();
}

PropertiesModel::~PropertiesModel() = default;


QVariant PropertiesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return {};

    std::string propertyName = propertiesList[index.row()];

    switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            if (index.column() == 0)
                return QString::fromStdString(propertyName);
            else if (index.column() == 1)
                return QString::fromStdString(selectedItem->GetPropertyValue(propertyName).value().ToString());
        // case Qt::DecorationRole:
        //     return iconOf(item->GetClass());
    }

    return {};
}

bool PropertiesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.column() != 1 && role != Qt::EditRole) return false;

    selectedItem->SetPropertyValue(propertiesList[index.row()], value.toString().toStdString());
    return true;
}

Qt::ItemFlags PropertiesModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == 0)
        return Qt::ItemIsEnabled;

    if (index.column() == 1)
        return Qt::ItemIsEnabled | Qt::ItemIsEditable;

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
    return !parent.isValid() ? selectedItem->GetProperties().size() : 0;
}

int PropertiesModel::columnCount(const QModelIndex &parent) const {
    return 2;
}