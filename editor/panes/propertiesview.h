#pragma once

#include "objects/base/instance.h"
#include "qtreeview.h"

class Ui_MainWindow;

class PropertiesView : public QTreeView {
public:
    PropertiesView(QWidget* parent = nullptr);
    ~PropertiesView() override;

    void setSelected(std::optional<InstanceRef> instance);
};