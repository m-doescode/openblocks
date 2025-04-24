#pragma once

#include "mainglwidget.h"
#include <qmdisubwindow.h>
#include <QBasicTimer>

enum RunState {
    RUN_STOPPED,
    RUN_RUNNING,
    RUN_PAUSED
};

class PlaceDocument : public QMdiSubWindow {
    QBasicTimer timer;
    RunState _runState;

    void timerEvent(QTimerEvent*) override;
public:
    MainGLWidget* placeWidget;
    PlaceDocument(QWidget* parent = nullptr);
    ~PlaceDocument() override;

    inline RunState runState() { return _runState; };
    void setRunState(RunState);
  
    void closeEvent(QCloseEvent *closeEvent) override;
    void init();
};