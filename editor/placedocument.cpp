#include "placedocument.h"
#include "common.h"
#include "mainglwidget.h"
#include "mainwindow.h"
#include "objects/joint/snap.h"
#include "objects/script.h"
#include "objects/script/scriptcontext.h"
#include "enum/surface.h"
#include <cstdio>
#include <memory>
#include <qboxlayout.h>
#include <qdebug.h>
#include <qevent.h>
#include <qmargins.h>
#include <qmdisubwindow.h>
#include <qlayout.h>
#include <qmimedata.h>

PlaceDocument::PlaceDocument(QWidget* parent):
    QMdiSubWindow(parent) {
    placeWidget = new MainGLWidget;
    setAcceptDrops(true);
    setWidget(placeWidget);
    setWindowTitle("Place");

    _runState = RUN_STOPPED;
}

PlaceDocument::~PlaceDocument() {
}

void PlaceDocument::setRunState(RunState newState) {
    if (newState == RUN_RUNNING && _runState != RUN_RUNNING) {
        if (_runState == RUN_PAUSED) {
            _runState = RUN_RUNNING;
            return;
        }

        _runState = RUN_RUNNING;

        std::shared_ptr<DataModel> newModel = editModeDataModel->CloneModel();
        gDataModel = newModel;
        gDataModel->Init(true);
    } else if (newState == RUN_PAUSED && _runState == RUN_RUNNING) {
        _runState = RUN_PAUSED;
    } else if (newState == RUN_STOPPED) {
        _runState = RUN_STOPPED;

        // GC: Check to make sure gDataModel gets properly garbage collected prior to this
        gDataModel = editModeDataModel;
    }
}

void PlaceDocument::closeEvent(QCloseEvent *closeEvent) {
    // Placeholder
    closeEvent->ignore();
}

std::shared_ptr<Part> shit;
static std::chrono::time_point lastTime = std::chrono::steady_clock::now();
void PlaceDocument::timerEvent(QTimerEvent* evt) {
    if (evt->timerId() != timer.timerId()) {
        QWidget::timerEvent(evt);
        return;
    }

    float deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - lastTime).count();
    lastTime = std::chrono::steady_clock::now();

    if (_runState == RUN_RUNNING)
        gWorkspace()->PhysicsStep(deltaTime);
    placeWidget->repaint();
    placeWidget->updateCycle();
    gDataModel->GetService<ScriptContext>()->RunSleepingThreads();
}


void PlaceDocument::init() {
    timer.start(33, this);
    placeWidget->buildContextMenu();

    std::shared_ptr<Part> lastPart;
    // Baseplate
    gWorkspace()->AddChild(lastPart = Part::New({
        .position = glm::vec3(0, -5, 0),
        .rotation = glm::vec3(0),
        .size = glm::vec3(512, 1.2, 512),
        .color = glm::vec3(0.388235, 0.372549, 0.384314),
        .anchored = true,
        .locked = true,
    }));
    lastPart->name = "Baseplate";
    gWorkspace()->SyncPartPhysics(lastPart);

    gWorkspace()->AddChild(lastPart = Part::New({
        .position = glm::vec3(-3.8),
        .rotation = glm::vec3(0),
        .size = glm::vec3(4, 1.2, 2),
        .color = glm::vec3(0.639216f, 0.635294f, 0.647059f),
    }));
    gWorkspace()->SyncPartPhysics(lastPart);
    auto part0 = lastPart;

    lastPart = Part::New();
}

void PlaceDocument::dragEnterEvent(QDragEnterEvent* evt) {
    // https://stackoverflow.com/a/14895393/16255372
    if (evt->mimeData()->hasUrls()) {
        evt->acceptProposedAction();
    }
}

void PlaceDocument::dropEvent(QDropEvent* evt) {
    auto urls = evt->mimeData()->urls();
    if (urls.size() == 0) return;
    QString fileName = urls[0].toLocalFile();
    MainWindow* mainWnd = dynamic_cast<MainWindow*>(window());
    mainWnd->openFile(fileName.toStdString());
}