#include "placedocument.h"
#include "common.h"
#include "datatypes/variant.h"
#include "mainglwidget.h"
#include "mainwindow.h"
#include "objects/service/script/scriptcontext.h"
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

class PlaceDocumentPhysicsWorker {
public:
    std::mutex sync;
    std::thread thread;
    std::condition_variable runningCond;
    bool running = false;
    bool quit = false;

    PlaceDocumentPhysicsWorker() : thread(&PlaceDocumentPhysicsWorker::doWork, this) {}
private:
    tu_time_t lastTime = tu_clock_micros();
    void doWork() {
        do {
            tu_time_t deltaTime = tu_clock_micros() - lastTime;
            lastTime = tu_clock_micros();

            // First frame is always empty
            if (deltaTime > 100) {
                gWorkspace()->PhysicsStep(float(deltaTime)/1'000'000);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(33 - deltaTime/1000));

            std::unique_lock lock(sync);
            runningCond.wait(lock, [&]{ return running || quit; });
            lock.unlock();
        } while (!quit);
    }
};

PlaceDocument::PlaceDocument(QWidget* parent):
    QMdiSubWindow(parent) {
    placeWidget = new MainGLWidget;
    setAcceptDrops(true);
    setWidget(placeWidget);
    setWindowTitle("Place");

    _runState = RUN_STOPPED;
    updateSelectionListeners(gDataModel->GetService<Selection>());

    worker = new PlaceDocumentPhysicsWorker();
}

PlaceDocument::~PlaceDocument() {
    worker->quit = true;
    worker->runningCond.notify_all();
    worker->thread.join();
}

void PlaceDocument::updatePhysicsWorker() {
    {
        std::lock_guard lock(worker->sync);
        worker->running = _runState == RUN_RUNNING;
    }
    worker->runningCond.notify_all();
}

void PlaceDocument::setRunState(RunState newState) {
    if (newState == RUN_RUNNING && _runState != RUN_RUNNING) {
        if (_runState == RUN_PAUSED) {
            _runState = RUN_RUNNING;
            updatePhysicsWorker();
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

        // TODO: GC: Check to make sure gDataModel gets properly garbage collected prior to this
        gDataModel = editModeDataModel;
        updateSelectionListeners(gDataModel->GetService<Selection>());
    }

    updatePhysicsWorker();
}

void PlaceDocument::updateSelectionListeners(std::shared_ptr<Selection> selection) {
    MainWindow* mainWnd = dynamic_cast<MainWindow*>(window());

    if (!selectionConnection.expired())
        selectionConnection.lock()->Disconnect();

    selectionConnection = selection->SelectionChanged->Connect([selection, mainWnd](std::vector<Variant> _){
        // Update properties
        if (selection->Get().size() != 1)
            mainWnd->ui->propertiesView->setSelected(std::nullopt);
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

std::shared_ptr<Part> shit;
void PlaceDocument::timerEvent(QTimerEvent* evt) {
    if (evt->timerId() != timer.timerId()) {
        QWidget::timerEvent(evt);
        return;
    }

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