#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QThread>
#include <QTimerEvent>
#include <QMouseEvent>
#include <QWidget>

#include "common.h"
#include "physics/simulation.h"

#include "wayland-pointer-constraints-unstable-v1-client-protocol.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer.start(33, this);
    setMouseTracking(true);
}

std::chrono::time_point lastTime = std::chrono::steady_clock::now();
void MainWindow::timerEvent(QTimerEvent* evt) {
    if (evt->timerId() != timer.timerId()) {
        QWidget::timerEvent(evt);
        return;
    }

    float deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - lastTime).count();
    lastTime = std::chrono::steady_clock::now();

    physicsStep(deltaTime);
    ui->mainWidget->update();
}

void MainWindow::updateMainWidget() {
    ui->mainWidget->update();
}

MainWindow::~MainWindow()
{
    delete ui;
}
