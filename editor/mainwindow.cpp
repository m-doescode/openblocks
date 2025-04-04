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
#include <functional>
#include <memory>
#include <optional>
#include <qcoreapplication.h>
#include <qglobal.h>
#include <qicon.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qwindowdefs.h>
#include <reactphysics3d/engine/PhysicsCommon.h>
#include <reactphysics3d/engine/PhysicsWorld.h>
#include <sstream>

#include "common.h"
#include "editorcommon.h"
#include "logger.h"
#include "objects/base/instance.h"
#include "objects/datamodel.h"
#include "objects/handles.h"
#include "physics/simulation.h"
#include "objects/part.h"
#include "qfiledialog.h"
#include "qclipboard.h"
#include "qmimedata.h"
#include "qobject.h"
#include "qsysinfo.h"

#ifdef _NDEBUG
#define NDEBUG
#endif

bool simulationPlaying = false;

bool worldSpaceTransforms = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    dataModel->Init();

    ui->setupUi(this);
    timer.start(33, this);
    setMouseTracking(true);

    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    connect(ui->actionQuit, &QAction::triggered, [&]() {
        this->close();
    });

    // Logger

    Logger::addLogListener(std::bind(&MainWindow::handleLog, this, std::placeholders::_1, std::placeholders::_2));

    QFont font("");
    font.setStyleHint(QFont::Monospace);
    ui->outputTextView->setFont(font);

    ui->outputTextView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->outputTextView, &QWidget::customContextMenuRequested, [&](QPoint point) {
        QMenu *menu = ui->outputTextView->createStandardContextMenu(point);

        menu->addAction("Clear Output", [&]() {
            ui->outputTextView->clear();
        });

        menu->exec(ui->outputTextView->mapToGlobal(point));
        delete menu;
    });

    // Explorer View

    ui->explorerView->buildContextMenu();

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

    connect(ui->actionToggleSpace, &QAction::triggered, this, [&]() {
        worldSpaceTransforms = !worldSpaceTransforms;
        updateToolbars();
        if (worldSpaceTransforms) {
            ui->actionToggleSpace->setText("W");
        } else {
            ui->actionToggleSpace->setText("L");
        }
    });

    connect(ui->actionNew, &QAction::triggered, this, [&]() {
        // Don't ask for confirmation if running a debug build (makes development easier)
        #ifdef NDEBUG
        // Ask if the user wants to save their changes
        // https://stackoverflow.com/a/33890731
        QMessageBox msgBox;
        msgBox.setText("Save changes before creating new document?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int result = msgBox.exec();

        if (result == QMessageBox::Cancel) return;
        if (result == QMessageBox::Save) {
            std::optional<std::string> path;
            if (!dataModel->HasFile())
                path = openFileDialog("Openblocks Level (*.obl)", ".obl", QFileDialog::AcceptSave, QString::fromStdString("Save " + dataModel->name));
            if (!path || path == "") return;
    
            dataModel->SaveToFile(path);
        }
        #endif

        // TODO: Also remove this (the reason this is here is because all parts are expected to
        // be deleted before their parent DataModel is. So it expects rigidBody to not be null, and tries
        // to destroy it, causing a segfault since it's already been destroyed. This is important for the later code.
        // TL;DR: This stinks and I need to fix it.)
        ui->mainWidget->lastPart = Part::New();

        dataModel = DataModel::New();
        dataModel->Init();
        ui->explorerView->updateRoot(dataModel);

        // TODO: Remove this and use a proper fix. This *WILL* cause a leak and memory issues in the future
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
    });

    connect(ui->actionSave, &QAction::triggered, this, [&]() {
        std::optional<std::string> path;
        if (!dataModel->HasFile())
            path = openFileDialog("Openblocks Level (*.obl)", ".obl", QFileDialog::AcceptSave, QString::fromStdString("Save " + dataModel->name));
        if (!path || path == "") return;

        dataModel->SaveToFile(path);
    });

    connect(ui->actionSaveAs, &QAction::triggered, this, [&]() {
        std::optional<std::string> path = openFileDialog("Openblocks Level (*.obl)", ".obl", QFileDialog::AcceptSave, QString::fromStdString("Save as " + dataModel->name));
        if (!path || path == "") return;

        dataModel->SaveToFile(path);
    });

    connect(ui->actionOpen, &QAction::triggered, this, [&]() {
        std::optional<std::string> path = openFileDialog("Openblocks Level (*.obl)", ".obl", QFileDialog::AcceptOpen);
        if (!path || path == "") return;
        std::shared_ptr<DataModel> newModel = DataModel::LoadFromFile(path.value());
        dataModel = newModel;
        ui->explorerView->updateRoot(newModel);
    });

    connect(ui->actionDelete, &QAction::triggered, this, [&]() {
        for (InstanceRefWeak inst : getSelection()) {
            if (inst.expired()) continue;
            inst.lock()->SetParent(std::nullopt);
        }
        setSelection(std::vector<InstanceRefWeak> {});
    });

    connect(ui->actionCopy, &QAction::triggered, this, [&]() {
        pugi::xml_document rootDoc;
        for (InstanceRefWeak inst : getSelection()) {
            if (inst.expired()) continue;
            inst.lock()->Serialize(&rootDoc);
        }

        std::ostringstream encoded;
        rootDoc.save(encoded);

        QMimeData* mimeData = new QMimeData;
        mimeData->setData("application/xml", QByteArray::fromStdString(encoded.str()));
        QApplication::clipboard()->setMimeData(mimeData);
    });
    connect(ui->actionCut, &QAction::triggered, this, [&]() {
        pugi::xml_document rootDoc;
        for (InstanceRefWeak inst : getSelection()) {
            if (inst.expired()) continue;
            inst.lock()->Serialize(&rootDoc);
            inst.lock()->SetParent(std::nullopt);
        }

        std::ostringstream encoded;
        rootDoc.save(encoded);

        QMimeData* mimeData = new QMimeData;
        mimeData->setData("application/xml", QByteArray::fromStdString(encoded.str()));
        QApplication::clipboard()->setMimeData(mimeData);
    });

    connect(ui->actionPaste, &QAction::triggered, this, [&]() {
        const QMimeData* mimeData = QApplication::clipboard()->mimeData();
        if (!mimeData || !mimeData->hasFormat("application/xml")) return;
        QByteArray bytes = mimeData->data("application/xml");
        std::string encoded = bytes.toStdString();

        pugi::xml_document rootDoc;
        rootDoc.load_string(encoded.c_str());

        for (pugi::xml_node instNode : rootDoc.children()) {
            InstanceRef inst = Instance::Deserialize(&instNode);
            workspace()->AddChild(inst);
        }
    });

    connect(ui->actionPasteInto, &QAction::triggered, this, [&]() {
        if (getSelection().size() != 1 || getSelection()[0].expired()) return;

        InstanceRef selectedParent = getSelection()[0].lock();

        const QMimeData* mimeData = QApplication::clipboard()->mimeData();
        if (!mimeData || !mimeData->hasFormat("application/xml")) return;
        QByteArray bytes = mimeData->data("application/xml");
        std::string encoded = bytes.toStdString();

        pugi::xml_document rootDoc;
        rootDoc.load_string(encoded.c_str());

        for (pugi::xml_node instNode : rootDoc.children()) {
            InstanceRef inst = Instance::Deserialize(&instNode);
            selectedParent->AddChild(inst);
        }
    });

    connect(ui->actionSaveModel, &QAction::triggered, this, [&]() {
        std::optional<std::string> path = openFileDialog("Openblocks Model (*.obm)", ".obm", QFileDialog::AcceptSave);
        if (!path) return;
        std::ofstream outStream(path.value());

        // Serialized XML for exporting
        pugi::xml_document modelDoc;
        pugi::xml_node modelRoot = modelDoc.append_child("openblocks");

        for (InstanceRefWeak inst : getSelection()) {
            if (inst.expired()) continue;
            inst.lock()->Serialize(&modelRoot);
        }

        modelDoc.save(outStream);
    });

    connect(ui->actionInsertModel, &QAction::triggered, this, [&]() {
        if (getSelection().size() != 1 || getSelection()[0].expired()) return;
        InstanceRef selectedParent = getSelection()[0].lock();

        std::optional<std::string> path = openFileDialog("Openblocks Model (*.obm)", ".obm", QFileDialog::AcceptOpen);
        if (!path) return;
        std::ifstream inStream(path.value());

        pugi::xml_document modelDoc;
        modelDoc.load(inStream);

        for (pugi::xml_node instNode : modelDoc.child("openblocks").children("Item")) {
            InstanceRef inst = Instance::Deserialize(&instNode);
            selectedParent->AddChild(inst);
        }
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

void MainWindow::closeEvent(QCloseEvent* evt) {
    #ifdef NDEBUG
    // Ask if the user wants to save their changes
    // https://stackoverflow.com/a/33890731
    QMessageBox msgBox;
    msgBox.setText("Save changes before creating new document?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int result = msgBox.exec();

    if (result == QMessageBox::Cancel) return evt->ignore();
    if (result == QMessageBox::Save) {
        std::optional<std::string> path;
        if (!dataModel->HasFile())
            path = openFileDialog("Openblocks Level (*.obl)", ".obl", QFileDialog::AcceptSave, QString::fromStdString("Save " + dataModel->name));
        if (!path || path == "") return evt->ignore();

        dataModel->SaveToFile(path);
    }
    #endif
}

void MainWindow::handleLog(Logger::LogLevel logLevel, std::string message) {
    if (logLevel == Logger::LogLevel::DEBUG) return;

    if (logLevel == Logger::LogLevel::INFO)
        ui->outputTextView->appendHtml(QString("<p>%1</p>").arg(QString::fromStdString(message)));
    if (logLevel == Logger::LogLevel::WARNING)
        ui->outputTextView->appendHtml(QString("<p style=\"color:rgb(255, 127, 0); font-weight: bold;\">%1</p>").arg(QString::fromStdString(message)));
    if (logLevel == Logger::LogLevel::ERROR || logLevel == Logger::LogLevel::FATAL_ERROR)
        ui->outputTextView->appendHtml(QString("<p style=\"color:rgb(255, 0, 0); font-weight: bold;\">%1</p>").arg(QString::fromStdString(message)));
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

    editorToolHandles->worldMode = (selectedTool == SelectedTool::SCALE || selectedTool == SelectedTool::ROTATE) ? false : worldSpaceTransforms;
    editorToolHandles->nixAxes = selectedTool == SelectedTool::ROTATE;

    editorToolHandles->active = selectedTool != SelectedTool::SELECT;
    editorToolHandles->handlesType =
      selectedTool == SelectedTool::MOVE ? HandlesType::MoveHandles
    : selectedTool == SelectedTool::SCALE ? HandlesType::ScaleHandles
    : selectedTool == SelectedTool::ROTATE ? HandlesType::RotateHandles
    : HandlesType::ScaleHandles;
}

std::optional<std::string> MainWindow::openFileDialog(QString filter, QString defaultExtension, QFileDialog::AcceptMode acceptMode, QString title) {
    QFileDialog dialog(this);
    if (title != "") dialog.setWindowTitle(title);
    dialog.setNameFilters(QStringList { filter, "All Files (*)" });
    dialog.setDefaultSuffix(defaultExtension);
    dialog.setAcceptMode(acceptMode);
    if (!dialog.exec())
        return std::nullopt;

    return dialog.selectedFiles().front().toStdString();
}

MainWindow::~MainWindow()
{
    delete ui;
}
