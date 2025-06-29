#pragma once

#include "datatypes/signal.h"
#include "mainglwidget.h"
#include <qevent.h>
#include <qmdisubwindow.h>
#include <QBasicTimer>

class Selection;

enum RunState {
    RUN_STOPPED,
    RUN_RUNNING,
    RUN_PAUSED
};

class PlaceDocument : public QMdiSubWindow {
    QBasicTimer timer;
    RunState _runState;

    std::weak_ptr<SignalConnection> selectionConnection;

    void timerEvent(QTimerEvent*) override;
    void updateSelectionListeners(std::shared_ptr<Selection>);
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