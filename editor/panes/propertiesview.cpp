#include "propertiesview.h"
#include "propertiesmodel.h"
#include "qaction.h"

PropertiesView::PropertiesView(QWidget* parent):
    QTreeView(parent) {
    this->setStyleSheet(QString("QTreeView::branch { border: none; }"));
}

PropertiesView::~PropertiesView() {
}

void PropertiesView::setSelected(std::optional<InstanceRef> instance) {
    if (instance.has_value()) {
        this->setModel(new PropertiesModel(instance.value()));
    } else {
        if (this->model()) delete this->model();
        this->setModel(nullptr);
    }
}