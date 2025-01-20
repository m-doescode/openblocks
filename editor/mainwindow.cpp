#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QThread>
#include <QTimerEvent>
#include <QMouseEvent>
#include <QWidget>
#include <QTreeView>
#include <QAbstractItemView>
#include <memory>
#include <optional>

#include "common.h"
#include "physics/simulation.h"
#include "objects/part.h"
#include "explorermodel.h"

#include "qabstractitemview.h"
#include "qevent.h"
#include "qnamespace.h"
#include "qobject.h"

class ExplorerEventFilter : public QObject {

private:
    QTreeView* explorerView;
    ExplorerModel* model;

    bool keyPress(QObject* object, QKeyEvent *event) {
        switch (event->key()) {
        case Qt::Key_Delete:
            QModelIndexList selectedIndexes = explorerView->selectionModel()->selectedIndexes();
            for (QModelIndex index : selectedIndexes) {
                model->fromIndex(index)->SetParent(std::nullopt);
            }
            break;
        }

        return QObject::eventFilter(object, event);
    }

    bool eventFilter(QObject *object, QEvent *event) {
        if (event->type() == QEvent::KeyPress)
            return keyPress(object, dynamic_cast<QKeyEvent*>(event));
        return QObject::eventFilter(object, event);
    }
public:
    ExplorerEventFilter(QTreeView* explorerView, ExplorerModel* model): explorerView(explorerView), model(model) {}
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , explorerModel(ExplorerModel(std::dynamic_pointer_cast<Instance>(workspace)))
{
    ui->setupUi(this);
    timer.start(33, this);
    setMouseTracking(true);

    ui->explorerView->setModel(&explorerModel);
    ui->explorerView->setRootIsDecorated(false);
    ui->explorerView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->explorerView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->explorerView->setDragEnabled(true);
    ui->explorerView->setAcceptDrops(true);
    ui->explorerView->setDropIndicatorShown(true);
    ui->explorerView->installEventFilter(new ExplorerEventFilter(ui->explorerView, &explorerModel));

    simulationInit();

    // Baseplate
    workspace->AddChild(ui->mainWidget->lastPart = Part::New({
        .position = glm::vec3(0, -5, 0),
        .rotation = glm::vec3(0),
        .scale = glm::vec3(512, 1.2, 512),
        .material = Material {
            .diffuse = glm::vec3(0.388235, 0.372549, 0.384314),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 32.0f,
        },
        .anchored = true,
    }));
    ui->mainWidget->lastPart->name = "Baseplate";
    syncPartPhysics(ui->mainWidget->lastPart);

    workspace->AddChild(ui->mainWidget->lastPart = Part::New({
        .position = glm::vec3(0),
        .rotation = glm::vec3(0),
        .scale = glm::vec3(4, 1.2, 2),
        .material = Material {
            .diffuse = glm::vec3(0.639216f, 0.635294f, 0.647059f),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 32.0f,
        }
    }));
    syncPartPhysics(ui->mainWidget->lastPart);
}

static std::chrono::time_point lastTime = std::chrono::steady_clock::now();
void MainWindow::timerEvent(QTimerEvent* evt) {
    if (evt->timerId() != timer.timerId()) {
        QWidget::timerEvent(evt);
        return;
    }

    float deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - lastTime).count();
    lastTime = std::chrono::steady_clock::now();

    physicsStep(deltaTime);
    ui->mainWidget->update();
    ui->mainWidget->updateCycle();
}

MainWindow::~MainWindow()
{
    delete ui;
}
