#pragma once

#include "objects/base/instance.h"
#include "objects/part.h"
#include "qevent.h"
#include "qmenu.h"
#include "qnamespace.h"
#include "qtreeview.h"
#include <QOpenGLWidget>
#include <QWidget>
#include <memory>
#include "explorermodel.h"

class Ui_MainWindow;

class PropertiesView : public QTreeView {
public:
    PropertiesView(QWidget* parent = nullptr);
    ~PropertiesView() override;

    void setSelected(std::optional<InstanceRef> instance);
};