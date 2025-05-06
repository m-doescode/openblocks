#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "common.h"
#include "logger.h"
#include "objects/datamodel.h"
#include "placedocument.h"
#include "script/scriptdocument.h"
#include <memory>
#include <qclipboard.h>
#include <qglobal.h>
#include <qmessagebox.h>
#include <qmimedata.h>
#include <qnamespace.h>
#include <qstylefactory.h>
#include <qstylehints.h>
#include <qmdisubwindow.h>
#include <pugixml.hpp>

#ifdef _NDEBUG
#define NDEBUG
#endif

bool worldSpaceTransforms = false;

inline bool isDarkMode() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const auto scheme = QGuiApplication::styleHints()->colorScheme();
    return scheme == Qt::ColorScheme::Dark;
#else
    const QPalette defaultPalette;
    const auto text = defaultPalette.color(QPalette::WindowText);
    const auto window = defaultPalette.color(QPalette::Window);
    return text.lightness() > window.lightness();
#endif // QT_VERSION
}

QtMessageHandler defaultMessageHandler = nullptr;

// std::map<QtMsgType, Logger::LogLevel> QT_MESSAGE_TYPE_TO_LOG_LEVEL = {
//     { QtMsgType::QtInfoMsg, Logger::LogLevel::INFO },  
//     { QtMsgType::QtSystemMsg, Logger::LogLevel::INFO }, 
//     { QtMsgType::QtDebugMsg, Logger::LogLevel::DEBUG }, 
//     { QtMsgType::QtWarningMsg, Logger::LogLevel::WARNING }, 
//     { QtMsgType::QtCriticalMsg, Logger::LogLevel::ERROR }, 
//     { QtMsgType::QtFatalMsg, Logger::LogLevel::FATAL_ERROR }, 
// };

void logQtMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    // Logger::log("[Qt] " + msg.toStdString(), QT_MESSAGE_TYPE_TO_LOG_LEVEL[type]);
    Logger::LogLevel logLevel = type == QtMsgType::QtFatalMsg ? Logger::LogLevel::FATAL_ERROR : Logger::LogLevel::DEBUG;
    Logger::log("[Qt] " + msg.toStdString(), logLevel);

    // if (defaultMessageHandler) defaultMessageHandler(type, context, msg);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    gDataModel->Init();

    ui->setupUi(this);
    setMouseTracking(true);

    // https://stackoverflow.com/a/78854851/16255372
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() + QStringList { "./assets/icons" });
    if (isDarkMode())
        QIcon::setFallbackThemeName("editor-dark");
    else
        QIcon::setFallbackThemeName("editor");

    // qApp->setStyle(QStyleFactory::create("fusion"));
    defaultMessageHandler = qInstallMessageHandler(logQtMessage);

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

    connectActionHandlers();
    
    // Update handles
    addSelectionListener([&](auto oldSelection, auto newSelection, bool fromExplorer) {
        editorToolHandles->adornee = {};
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

    addSelectionListener([&](auto oldSelection, auto newSelection, bool __) {
        for (InstanceRefWeak inst : oldSelection) {
            if (inst.expired() || inst.lock()->GetClass() != &Part::TYPE) continue;
            std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst.lock());
            part->selected = false;
        }

        for (InstanceRefWeak inst : newSelection) {
            if (inst.expired() || inst.lock()->GetClass() != &Part::TYPE) continue;
            std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst.lock());
            part->selected = true;
        }
    });

    // ui->explorerView->Init(ui);
    placeDocument = new PlaceDocument(this);
    placeDocument->setAttribute(Qt::WA_DeleteOnClose, true);
    ui->mdiArea->addSubWindow(placeDocument);
    ui->mdiArea->currentSubWindow()->showMaximized();
    ui->mdiArea->findChild<QTabBar*>()->setExpanding(false);
    placeDocument->init();

    ui->mdiArea->setTabsClosable(true);
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
        if (!gDataModel->HasFile())
            path = openFileDialog("Openblocks Level (*.obl)", ".obl", QFileDialog::AcceptSave, QString::fromStdString("Save " + gDataModel->name));
        if (!path || path == "") return evt->ignore();

        gDataModel->SaveToFile(path);
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

void MainWindow::connectActionHandlers() {
    // Explorer View

    ui->explorerView->buildContextMenu();

    connect(ui->actionToolSelect, &QAction::triggered, this, [&]() { selectedTool = TOOL_SELECT; updateToolbars(); });
    connect(ui->actionToolMove, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_MOVE : TOOL_SELECT; updateToolbars(); });
    connect(ui->actionToolScale, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_SCALE : TOOL_SELECT; updateToolbars(); });
    connect(ui->actionToolRotate, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_ROTATE : TOOL_SELECT; updateToolbars(); });
    
    connect(ui->actionToolSmooth, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_SMOOTH : TOOL_SELECT; updateToolbars(); });
    connect(ui->actionToolGlue, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_GLUE : TOOL_SELECT; updateToolbars(); });
    connect(ui->actionToolWeld, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_WELD : TOOL_SELECT; updateToolbars(); });
    connect(ui->actionToolStuds, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_STUDS : TOOL_SELECT; updateToolbars(); });
    connect(ui->actionToolInlets, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_INLETS : TOOL_SELECT; updateToolbars(); });
    connect(ui->actionToolUniversal, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_UNIVERSAL : TOOL_SELECT; updateToolbars(); });
    connect(ui->actionToolHinge, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_HINGE : TOOL_SELECT; updateToolbars(); });
    connect(ui->actionToolMotor, &QAction::triggered, this, [&](bool state) { selectedTool = state ? TOOL_MOTOR : TOOL_SELECT; updateToolbars(); });
    ui->actionToolSelect->setChecked(true);
    selectedTool = TOOL_SELECT;

    connect(ui->actionGridSnap1, &QAction::triggered, this, [&]() { snappingMode = GridSnappingMode::SNAP_1_STUD; updateToolbars(); });
    connect(ui->actionGridSnap05, &QAction::triggered, this, [&]() { snappingMode = GridSnappingMode::SNAP_05_STUDS; updateToolbars(); });
    connect(ui->actionGridSnapOff, &QAction::triggered, this, [&]() { snappingMode = GridSnappingMode::SNAP_OFF; updateToolbars(); });
    ui->actionGridSnap1->setChecked(true);
    snappingMode = GridSnappingMode::SNAP_1_STUD;

    connect(ui->actionToggleEditSounds, &QAction::triggered, this, [&](bool state) {
        editSoundEffects = state;
        ui->actionToggleEditSounds->setIcon(QIcon::fromTheme(editSoundEffects ? "audio-volume-high" : "audio-volume-muted"));
    });
    ui->actionToggleEditSounds->setChecked(true);

    connect(ui->actionRunSimulation, &QAction::triggered, this, [&]() {
        RunState prevState = placeDocument->runState();
        placeDocument->setRunState(RUN_RUNNING);

        if (prevState == RUN_STOPPED)
            ui->explorerView->updateRoot(gDataModel);
        updateToolbars();
    });

    connect(ui->actionPauseSimulation, &QAction::triggered, this, [&]() {
        placeDocument->setRunState(RUN_PAUSED);
        updateToolbars();
    });

    connect(ui->actionStopSimulation, &QAction::triggered, this, [&]() {
        placeDocument->setRunState(RUN_STOPPED);
        ui->explorerView->updateRoot(gDataModel);
        updateToolbars();
    });

    ui->actionRunSimulation->setEnabled(true);
    ui->actionPauseSimulation->setEnabled(false);
    ui->actionStopSimulation->setEnabled(false);

    connect(ui->actionToggleSpace, &QAction::triggered, this, [&]() {
        worldSpaceTransforms = !worldSpaceTransforms;
        updateToolbars();
        if (worldSpaceTransforms) {
            ui->actionToggleSpace->setText("World");
            ui->actionToggleSpace->setIcon(QIcon::fromTheme("space-global"));
        } else {
            ui->actionToggleSpace->setText("Local");
            ui->actionToggleSpace->setIcon(QIcon::fromTheme("space-local"));
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
            if (!gDataModel->HasFile())
                path = openFileDialog("Openblocks Level (*.obl)", ".obl", QFileDialog::AcceptSave, QString::fromStdString("Save " + gDataModel->name));
            if (!path || path == "") return;
    
            gDataModel->SaveToFile(path);
        }
        #endif

        // TODO: Also remove this (the reason this is here is because all parts are expected to
        // be deleted before their parent DataModel is. So it expects rigidBody to not be null, and tries
        // to destroy it, causing a segfault since it's already been destroyed. This is important for the later code.
        // TL;DR: This stinks and I need to fix it.)
        placeDocument->placeWidget->lastPart = Part::New();

        editModeDataModel = DataModel::New();
        gDataModel = editModeDataModel;
        gDataModel->Init();
        ui->explorerView->updateRoot(gDataModel);

        // Baseplate
        gWorkspace()->AddChild(placeDocument->placeWidget->lastPart = Part::New({
            .position = glm::vec3(0, -5, 0),
            .rotation = glm::vec3(0),
            .size = glm::vec3(512, 1.2, 512),
            .color = glm::vec3(0.388235, 0.372549, 0.384314),
            .anchored = true,
            .locked = true,
        }));
        placeDocument->placeWidget->lastPart->name = "Baseplate";
        gWorkspace()->SyncPartPhysics(placeDocument->placeWidget->lastPart);
    });

    connect(ui->actionSave, &QAction::triggered, this, [&]() {
        std::optional<std::string> path;
        if (!editModeDataModel->HasFile())
            path = openFileDialog("Openblocks Level (*.obl)", ".obl", QFileDialog::AcceptSave, QString::fromStdString("Save " + editModeDataModel->name));
        if (!editModeDataModel->HasFile() && (!path || path == "")) return;

        editModeDataModel->SaveToFile(path);
    });

    connect(ui->actionSaveAs, &QAction::triggered, this, [&]() {
        std::optional<std::string> path = openFileDialog("Openblocks Level (*.obl)", ".obl", QFileDialog::AcceptSave, QString::fromStdString("Save as " + editModeDataModel->name));
        if (!path || path == "") return;

        editModeDataModel->SaveToFile(path);
    });

    connect(ui->actionOpen, &QAction::triggered, this, [&]() {
        std::optional<std::string> path = openFileDialog("Openblocks Level (*.obl)", ".obl", QFileDialog::AcceptOpen);
        if (!path || path == "") return;
        
        // // See TODO: Also remove this (the reaso
        // ui->mainWidget->lastPart = Part::New();
        
        // simulationInit();
        std::shared_ptr<DataModel> newModel = DataModel::LoadFromFile(path.value());
        editModeDataModel = newModel;
        gDataModel = newModel;
        newModel->Init();
        ui->explorerView->updateRoot(newModel);

        // Reset running state
        placeDocument->setRunState(RUN_STOPPED);
        updateToolbars();
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
            inst.lock()->Serialize(rootDoc);
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
            inst.lock()->Serialize(rootDoc);
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
            result<InstanceRef, NoSuchInstance> inst = Instance::Deserialize(instNode);
            if (!inst) { inst.logError(); continue; }
            gWorkspace()->AddChild(inst.expect());
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
            result<InstanceRef, NoSuchInstance> inst = Instance::Deserialize(instNode);
            if (!inst) { inst.logError(); continue; }
            selectedParent->AddChild(inst.expect());
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
            inst.lock()->Serialize(modelRoot);
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
            result<InstanceRef, NoSuchInstance> inst = Instance::Deserialize(instNode);
            if (!inst) { inst.logError(); continue; }
            selectedParent->AddChild(inst.expect());
        }
    });
}

void MainWindow::updateToolbars() {
    ui->actionToolSelect->setChecked(selectedTool == TOOL_SELECT);
    ui->actionToolMove->setChecked(selectedTool == TOOL_MOVE);
    ui->actionToolScale->setChecked(selectedTool == TOOL_SCALE);
    ui->actionToolRotate->setChecked(selectedTool == TOOL_ROTATE);

    ui->actionToolSmooth->setChecked(selectedTool == TOOL_SMOOTH);
    ui->actionToolGlue->setChecked(selectedTool == TOOL_GLUE);
    ui->actionToolWeld->setChecked(selectedTool == TOOL_WELD);
    ui->actionToolStuds->setChecked(selectedTool == TOOL_STUDS);
    ui->actionToolInlets->setChecked(selectedTool == TOOL_INLETS);
    ui->actionToolUniversal->setChecked(selectedTool == TOOL_UNIVERSAL);
    ui->actionToolHinge->setChecked(selectedTool == TOOL_HINGE);
    ui->actionToolMotor->setChecked(selectedTool == TOOL_MOTOR);

    ui->actionGridSnap1->setChecked(snappingMode == GridSnappingMode::SNAP_1_STUD);
    ui->actionGridSnap05->setChecked(snappingMode == GridSnappingMode::SNAP_05_STUDS);
    ui->actionGridSnapOff->setChecked(snappingMode == GridSnappingMode::SNAP_OFF);

    editorToolHandles->worldMode = (selectedTool == TOOL_SCALE || selectedTool == TOOL_ROTATE) ? false : worldSpaceTransforms;
    editorToolHandles->nixAxes = selectedTool == TOOL_ROTATE;

    editorToolHandles->active = selectedTool > TOOL_SELECT && selectedTool < TOOL_SMOOTH;
    editorToolHandles->handlesType =
      selectedTool == TOOL_MOVE ? HandlesType::MoveHandles
    : selectedTool == TOOL_SCALE ? HandlesType::ScaleHandles
    : selectedTool == TOOL_ROTATE ? HandlesType::RotateHandles
    : HandlesType::ScaleHandles;

    ui->actionRunSimulation->setEnabled(placeDocument->runState() != RUN_RUNNING);
    ui->actionPauseSimulation->setEnabled(placeDocument->runState() == RUN_RUNNING);
    ui->actionStopSimulation->setEnabled(placeDocument->runState() != RUN_STOPPED);
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

void MainWindow::openScriptDocument(std::shared_ptr<Script> script) {
    ScriptDocument* doc = new ScriptDocument(script);
    doc->setAttribute(Qt::WA_DeleteOnClose, true);
    ui->mdiArea->addSubWindow(doc);
    ui->mdiArea->setActiveSubWindow(doc);
    doc->showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
}
