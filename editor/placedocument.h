#pragma once

#include "datatypes/signal.h"
#include "mainglwidget.h"
#include <condition_variable>
#include <mutex>
#include <qevent.h>
#include <qmdisubwindow.h>
#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <QBasicTimer>
#include <type_traits>

class Selection;
class PlaceDocumentPhysicsWorker;

enum RunState {
    RUN_STOPPED,
    RUN_RUNNING,
    RUN_PAUSED
};

class PlaceDocument : public QMdiSubWindow {
    QBasicTimer timer;
    RunState _runState;

    PlaceDocumentPhysicsWorker* worker;

    std::weak_ptr<SignalConnection> selectionConnection;

    void timerEvent(QTimerEvent*) override;
    void updateSelectionListeners(std::shared_ptr<Selection>);

    void updatePhysicsWorker();
public:
    MainGLWidget* placeWidget;
    PlaceDocument(QWidget* parent = nullptr);
    ~PlaceDocument() override;

    inline RunState runState() { return _runState; };
    void setRunState(RunState);
  
    void closeEvent(QCloseEvent *closeEvent) override;
    void init();
protected:
    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;
};