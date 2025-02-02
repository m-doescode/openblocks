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
#include <optional>

#include "common.h"
#include "editorcommon.h"
#include "physics/simulation.h"
#include "objects/part.h"
#include "qitemselectionmodel.h"
#include "qobject.h"

SelectedTool selectedTool;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer.start(33, this);
    setMouseTracking(true);

    connect(ui->explorerView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&](const QItemSelection &selected, const QItemSelection &deselected) {
        if (selected.count() == 0) return;

        std::optional<InstanceRef> inst = selected.count() == 0 ? std::nullopt
            : std::make_optional(((Instance*)selected.indexes()[0].internalPointer())->shared_from_this());

        ui->propertiesView->setSelected(inst);
    });

    connect(ui->actionToolSelect, &QAction::triggered, this, [&]() { selectedTool = SelectedTool::SELECT; updateSelectedTool(); });
    connect(ui->actionToolMove, &QAction::triggered, this, [&](bool state) { selectedTool = state ? SelectedTool::MOVE : SelectedTool::SELECT; updateSelectedTool(); });
    connect(ui->actionToolScale, &QAction::triggered, this, [&](bool state) { selectedTool = state ? SelectedTool::SCALE : SelectedTool::SELECT; updateSelectedTool(); });
    connect(ui->actionToolRotate, &QAction::triggered, this, [&](bool state) { selectedTool = state ? SelectedTool::ROTATE : SelectedTool::SELECT; updateSelectedTool(); });
    ui->actionToolSelect->setChecked(true);
    
    // ui->explorerView->Init(ui);

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

void MainWindow::updateSelectedTool() {
    ui->actionToolSelect->setChecked(selectedTool == SelectedTool::SELECT);
    ui->actionToolMove->setChecked(selectedTool == SelectedTool::MOVE);
    ui->actionToolScale->setChecked(selectedTool == SelectedTool::SCALE);
    ui->actionToolRotate->setChecked(selectedTool == SelectedTool::ROTATE);
}

MainWindow::~MainWindow()
{
    delete ui;
}
