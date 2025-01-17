#ifndef MAINGLWIDGET_H
#define MAINGLWIDGET_H

#include "objects/part.h"
#include "qevent.h"
#include <QOpenGLWidget>
#include <QWidget>
#include <memory>

class MainGLWidget : public QOpenGLWidget {
public:
    MainGLWidget(QWidget *parent = nullptr);
    void updateCycle();
    std::shared_ptr<Part> lastPart;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mouseMoveEvent(QMouseEvent* evt) override;
    void mousePressEvent(QMouseEvent* evt) override;
    void mouseReleaseEvent(QMouseEvent* evt) override;
    void keyPressEvent(QKeyEvent* evt) override;
    void keyReleaseEvent(QKeyEvent* evt) override;
};

#endif // MAINGLWIDGET_H
