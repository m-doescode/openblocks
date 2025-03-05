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
#include "editorcommon.h"
#include "objects/base/instance.h"
#include "objects/datamodel.h"
#include "objects/handles.h"
#include "physics/simulation.h"
#include "objects/part.h"
#include "qfiledialog.h"
#include "qitemselectionmodel.h"
#include "qobject.h"
#include "qsysinfo.h"

bool simulationPlaying = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    dataModel->Init();

    ui->setupUi(this);
    timer.start(33, this);
    setMouseTracking(true);

    ConnectSelectionChangeHandler();

    connect(ui->actionToolSelect, &QAction::triggered, this, [&]() { selectedTool = SelectedTool::SELECT; updateToolbars(); });
    connect(ui->actionToolMove, &QAction::triggered, this, [&](bool state) { selectedTool = state ? SelectedTool::MOVE : SelectedTool::SELECT; updateToolbars(); });
    connect(ui->actionToolScale, &QAction::triggered, this, [&](bool state) { selectedTool = state ? SelectedTool::SCALE : SelectedTool::SELECT; updateToolbars(); });
    connect(ui->actionToolRotate, &QAction::triggered, this, [&](bool state) { selectedTool = state ? SelectedTool::ROTATE : SelectedTool::SELECT; updateToolbars(); });
    ui->actionToolSelect->setChecked(true);
    selectedTool = SelectedTool::SELECT;

    connect(ui->actionGridSnap1, &QAction::triggered, this, [&]() { snappingMode = GridSnappingMode::SNAP_1_STUD; updateToolbars(); });
    connect(ui->actionGridSnap05, &QAction::triggered, this, [&]() { snappingMode = GridSnappingMode::SNAP_05_STUDS; updateToolbars(); });
    connect(ui->actionGridSnapOff, &QAction::triggered, this, [&]() { snappingMode = GridSnappingMode::SNAP_OFF; updateToolbars(); });
    ui->actionGridSnap1->setChecked(true);
    snappingMode = GridSnappingMode::SNAP_1_STUD;

    connect(ui->actionToggleSimulation, &QAction::triggered, this, [&]() {
        simulationPlaying = !simulationPlaying;
        if (simulationPlaying) {
            ui->actionToggleSimulation->setText("Pause simulation");
            ui->actionToggleSimulation->setToolTip("Pause the simulation");
            ui->actionToggleSimulation->setIcon(QIcon::fromTheme("media-playback-pause"));
        } else {
            ui->actionToggleSimulation->setText("Resume simulation");
            ui->actionToggleSimulation->setToolTip("Resume the simulation");
            ui->actionToggleSimulation->setIcon(QIcon::fromTheme("media-playback-start"));
        }
    });

    connect(ui->actionSave, &QAction::triggered, this, [&]() {
        std::optional<std::string> path;
        if (!dataModel->HasFile())
            path = QFileDialog::getSaveFileName(this, QString::fromStdString("Save " + dataModel->name), "", "*.obl").toStdString();
        if (path == "") return;

        dataModel->SaveToFile(path);
    });

    connect(ui->actionOpen, &QAction::triggered, this, [&]() {
        std::string path = QFileDialog::getOpenFileName(this, "Load file", "", "*.obl").toStdString();
        if (path == "") return;
        std::shared_ptr<DataModel> newModel = DataModel::LoadFromFile(path);
        dataModel = newModel;
        delete ui->explorerView->selectionModel();
        ui->explorerView->reset();
        ui->explorerView->setModel(new ExplorerModel(dataModel));
        ConnectSelectionChangeHandler();
    });
    
    // Update handles
    addSelectionListener([&](auto oldSelection, auto newSelection, bool fromExplorer) {
        editorToolHandles->adornee = std::nullopt;
        if (newSelection.size() == 0) return;
        InstanceRef inst = newSelection[0].lock();
        if (inst->GetClass() != &Part::TYPE) return;

        editorToolHandles->adornee = std::dynamic_pointer_cast<Part>(inst);
    });

    // Update properties
    addSelectionListener([&](auto oldSelection, auto newSelection, bool fromExplorer) {
        if (newSelection.size() == 0) return;
        if (newSelection.size() > 1)
            ui->propertiesView->setSelected(std::nullopt);
        ui->propertiesView->setSelected(newSelection[0].lock());
    });

    // ui->explorerView->Init(ui);

    simulationInit();

    // Baseplate
    workspace()->AddChild(ui->mainWidget->lastPart = Part::New({
        .position = glm::vec3(0, -5, 0),
        .rotation = glm::vec3(0),
        .size = glm::vec3(512, 1.2, 512),
        .color = glm::vec3(0.388235, 0.372549, 0.384314),
        .anchored = true,
    }));
    ui->mainWidget->lastPart->name = "Baseplate";
    syncPartPhysics(ui->mainWidget->lastPart);

    workspace()->AddChild(ui->mainWidget->lastPart = Part::New({
        .position = glm::vec3(0),
        .rotation = glm::vec3(0.5, 2, 1),
        .size = glm::vec3(4, 1.2, 2),
        .color = glm::vec3(0.639216f, 0.635294f, 0.647059f),
    }));
    syncPartPhysics(ui->mainWidget->lastPart);
}

void MainWindow::ConnectSelectionChangeHandler() {
    // connect(ui->explorerView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&](const QItemSelection &selected, const QItemSelection &deselected) {
    //     if (selected.count() == 0) return;

    //     std::optional<InstanceRef> inst = selected.count() == 0 ? std::nullopt
    //         : std::make_optional(((Instance*)selected.indexes()[0].internalPointer())->shared_from_this());

    //     ui->propertiesView->setSelected(inst);
    // });
}

static std::chrono::time_point lastTime = std::chrono::steady_clock::now();
void MainWindow::timerEvent(QTimerEvent* evt) {
    if (evt->timerId() != timer.timerId()) {
        QWidget::timerEvent(evt);
        return;
    }

    float deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - lastTime).count();
    lastTime = std::chrono::steady_clock::now();

    if (simulationPlaying)
        physicsStep(deltaTime);
    ui->mainWidget->update();
    ui->mainWidget->updateCycle();
}

void MainWindow::updateToolbars() {
    ui->actionToolSelect->setChecked(selectedTool == SelectedTool::SELECT);
    ui->actionToolMove->setChecked(selectedTool == SelectedTool::MOVE);
    ui->actionToolScale->setChecked(selectedTool == SelectedTool::SCALE);
    ui->actionToolRotate->setChecked(selectedTool == SelectedTool::ROTATE);

    ui->actionGridSnap1->setChecked(snappingMode == GridSnappingMode::SNAP_1_STUD);
    ui->actionGridSnap05->setChecked(snappingMode == GridSnappingMode::SNAP_05_STUDS);
    ui->actionGridSnapOff->setChecked(snappingMode == GridSnappingMode::SNAP_OFF);

    // editorToolHandles->worldMode = false;
    if (selectedTool == SelectedTool::MOVE) editorToolHandles->worldMode = true;
    if (selectedTool == SelectedTool::SCALE) editorToolHandles->worldMode = false;
    editorToolHandles->active = selectedTool != SelectedTool::SELECT;
    editorToolHandles->handlesType =
      selectedTool == SelectedTool::MOVE ? HandlesType::MoveHandles
    : selectedTool == SelectedTool::SCALE ? HandlesType::ScaleHandles
    : selectedTool == SelectedTool::ROTATE ? HandlesType::RotateHandles
    : HandlesType::ScaleHandles;
}

MainWindow::~MainWindow()
{
    delete ui;
}
