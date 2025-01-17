#include <GL/glew.h>
#include <chrono>

#include <QMouseEvent>

#include "GLFW/glfw3.h"
#include "qcursor.h"
#include "qevent.h"
#include "qnamespace.h"
#include "rendering/renderer.h"
#include "physics/simulation.h"
#include "camera.h"

#include "common.h"

#include "mainglwidget.h"

MainGLWidget::MainGLWidget(QWidget* parent): QOpenGLWidget(parent) {
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

void MainGLWidget::initializeGL() {
    glewInit();
    renderInit(NULL);
}

void MainGLWidget::resizeGL(int w, int h) {
    // Update projection matrix and other size related settings:
    // m_projection.setToIdentity();
    // m_projection.perspective(45.0f, w / float(h), 0.01f, 100.0f);
    // ...
}

void MainGLWidget::paintGL() {
    ::render(NULL);
}

bool isMouseDragging = false;
QPoint lastMousePos;
void MainGLWidget::mouseMoveEvent(QMouseEvent* evt) {
    // if (!(evt->buttons() & Qt::RightButton)) return;
    if (!isMouseDragging) return;
    
    camera.processRotation(evt->pos().x() - lastMousePos.x(), evt->pos().y() - lastMousePos.y());
    lastMousePos = evt->pos();
    // QCursor::setPos(lastMousePos);
}

void MainGLWidget::mousePressEvent(QMouseEvent* evt) {
    if (evt->button() != Qt::RightButton) return;

    lastMousePos = evt->pos();
    isMouseDragging = true;
}

void MainGLWidget::mouseReleaseEvent(QMouseEvent* evt) {
    isMouseDragging = false;
}

static int moveZ = 0;
static int moveX = 0;

static std::chrono::time_point lastTime = std::chrono::steady_clock::now();
void MainGLWidget::updateCycle() {
    float deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - lastTime).count();
    lastTime = std::chrono::steady_clock::now();

    if (moveZ)
        camera.processMovement(moveZ == 1 ? DIRECTION_FORWARD : DIRECTION_BACKWARDS, deltaTime);
    if (moveX)
        camera.processMovement(moveX == 1 ? DIRECTION_LEFT : DIRECTION_RIGHT, deltaTime);

}

void MainGLWidget::keyPressEvent(QKeyEvent* evt) {
    if (evt->key() == Qt::Key_W) moveZ = 1;
    else if (evt->key() == Qt::Key_S) moveZ = -1;
    
    if (evt->key() == Qt::Key_A) moveX = 1;
    else if (evt->key() == Qt::Key_D) moveX = -1;

    if (evt->key() == Qt::Key_F) {
        workspace->AddChild(lastPart = Part::New({
            .position = camera.cameraPos + camera.cameraFront * glm::vec3(3),
            .rotation = glm::vec3(0),
            .scale = glm::vec3(1, 1, 1),
            .material = Material {
                .diffuse = glm::vec3(1.0f, 0.5f, 0.31f),
                .specular = glm::vec3(0.5f, 0.5f, 0.5f),
                .shininess = 32.0f,
            }
        }));
        syncPartPhysics(lastPart);
    }
}

void MainGLWidget::keyReleaseEvent(QKeyEvent* evt) {
    if (evt->key() == Qt::Key_W || evt->key() == Qt::Key_S) moveZ = 0;
    else if (evt->key() == Qt::Key_A || evt->key() == Qt::Key_D) moveX = 0;
}