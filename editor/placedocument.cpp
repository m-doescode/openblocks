#include "placedocument.h"
#include "common.h"
#include "mainglwidget.h"
#include "objects/joint/snap.h"
#include "rendering/surface.h"
#include <cstdio>
#include <memory>
#include <qboxlayout.h>
#include <qevent.h>
#include <qmargins.h>
#include <qmdisubwindow.h>
#include <qlayout.h>

PlaceDocument::PlaceDocument(QWidget* parent):
    QMdiSubWindow(parent) {
    placeWidget = new MainGLWidget;
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
}


void PlaceDocument::init() {
    timer.start(33, this);

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
        .position = glm::vec3(0),
        .rotation = glm::vec3(-2.6415927, 1.1415926, 2.57075),
        .size = glm::vec3(4, 1.2, 2),
        .color = glm::vec3(0.639216f, 0.635294f, 0.647059f),
    }));
    gWorkspace()->SyncPartPhysics(lastPart);
    auto part0 = lastPart;

    gWorkspace()->AddChild(lastPart = Part::New({
        .position = glm::vec3(1.7610925, 0.48568499, -0.82623518),
        // .rotation = glm::vec3(0.5, 2, 1),
        .rotation = glm::vec3(-2.6415927, 1.1415926, -2.141639),
        .size = glm::vec3(4, 1.2, 2),
        .color = glm::vec3(0.639216f, 0.635294f, 0.647059f),
    }));
    gWorkspace()->SyncPartPhysics(lastPart);
    auto part1 = lastPart;

    lastPart = Part::New();
    shit = part1;

    part0->anchored = true;
    part0->UpdateProperty("Anchored");

    // auto snap = Snap::New();
    // snap->part0 = part0;
    // snap->part1 = part1;
    // snap->c0 = part1->cframe;
    // snap->c1 = part0->cframe;

    // gWorkspace()->AddChild(snap);
    // snap->UpdateProperty("Part0");
    // snap->UpdateProperty("Part1");

    // part0->backSurface = SurfaceWeld;
    // part1->frontSurface = SurfaceWeld;

    // part0->backSurface = SurfaceHinge;
    part0->backSurface = SurfaceMotor;
    // part1->frontSurface = SurfaceHinge;
}