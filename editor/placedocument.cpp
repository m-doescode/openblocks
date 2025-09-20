#include "placedocument.h"
#include "common.h"
#include "datatypes/variant.h"
#include "mainglwidget.h"
#include "mainwindow.h"
#include "objects/service/script/scriptcontext.h"
#include <chrono>
#include <memory>
#include <mutex>
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qdebug.h>
#include <qevent.h>
#include <qglobal.h>
#include <qmargins.h>
#include <qmdisubwindow.h>
#include <qlayout.h>
#include <qmimedata.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <thread>
#include "../ui_mainwindow.h"
#include "objects/service/selection.h"
#include "timeutil.h"

PlaceDocument::PlaceDocument(QWidget* parent):
    QMdiSubWindow(parent) {
    placeWidget = new MainGLWidget;
    setAcceptDrops(true);
    setWidget(placeWidget);
    setWindowTitle("Place");

    _runState = RUN_STOPPED;
    updateSelectionListeners(gDataModel->GetService<Selection>());
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
        updateSelectionListeners(gDataModel->GetService<Selection>());
    } else if (newState == RUN_PAUSED && _runState == RUN_RUNNING) {
        _runState = RUN_PAUSED;
    } else if (newState == RUN_STOPPED) {
        _runState = RUN_STOPPED;

#ifndef NDEBUG
    printf("DataModel stopped. Remaning use counts (should be 1): %ld\n", gDataModel.use_count());
#endif
        // TODO: GC: Check to make sure gDataModel gets properly garbage collected prior to this
        gDataModel = editModeDataModel;
        updateSelectionListeners(gDataModel->GetService<Selection>());
    }
}

void PlaceDocument::updateSelectionListeners(std::shared_ptr<Selection> selection) {
    MainWindow* mainWnd = dynamic_cast<MainWindow*>(window());

    if (!selectionConnection.expired())
        selectionConnection.lock()->Disconnect();

    selectionConnection = selection->SelectionChanged->Connect([selection, mainWnd](std::vector<Variant> _){
        // Update properties
        if (selection->Get().size() != 1)
            mainWnd->ui->propertiesView->setSelected(nullptr);
        else
            mainWnd->ui->propertiesView->setSelected(selection->Get()[0]);

        // Update explorer
        mainWnd->ui->explorerView->setSelectedObjects(selection->Get());
    });
}

void PlaceDocument::closeEvent(QCloseEvent *closeEvent) {
    // Placeholder
    closeEvent->ignore();
}

void PlaceDocument::timerEvent(QTimerEvent* evt) {
    if (evt->timerId() != timer.timerId()) {
        QWidget::timerEvent(evt);
        return;
    }

    placeWidget->repaint();
    placeWidget->updateCycle();
    if (_runState != RUN_RUNNING) return;
    gDataModel->GetService<ScriptContext>()->RunSleepingThreads();
    gDataModel->GetService<Workspace>()->PhysicsStep(0.033);
}


void PlaceDocument::init() {
    timer.start(33, this);
    placeWidget->buildContextMenu();

    std::shared_ptr<BasePart> lastPart;
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
    // gWorkspace()->SyncPartPhysics(lastPart);

    // gWorkspace()->AddChild(lastPart = Part::New({
    //     .position = glm::vec3(-3.8),
    //     .rotation = glm::vec3(0),
    //     .size = glm::vec3(4, 1.2, 2),
    //     .color = glm::vec3(0.639216f, 0.635294f, 0.647059f),
    // }));
    // gWorkspace()->SyncPartPhysics(lastPart);
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