#include <GL/glew.h>
#include <chrono>

#include <QMouseEvent>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <memory>
#include <optional>
#include <reactphysics3d/collision/RaycastInfo.h>
#include <vector>

#include "physics/util.h"
#include "qcursor.h"
#include "qevent.h"
#include "qnamespace.h"
#include "qwindowdefs.h"
#include "rendering/renderer.h"
#include "physics/simulation.h"
#include "camera.h"

#include "common.h"

#include "mainglwidget.h"

MainGLWidget::MainGLWidget(QWidget* parent): QOpenGLWidget(parent) {
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setMouseTracking(true);
}

void MainGLWidget::initializeGL() {
    glewInit();
    renderInit(NULL, width(), height());
}

extern int vpx, vpy;

void MainGLWidget::resizeGL(int w, int h) {
    // Update projection matrix and other size related settings:
    // m_projection.setToIdentity();
    // m_projection.perspective(45.0f, w / float(h), 0.01f, 100.0f);
    // ...
    // glViewport(0, 0, w, h);
    setViewport(w, h);
}

void MainGLWidget::paintGL() {
    ::render(NULL);
}

bool isMouseRightDragging = false;
QPoint lastMousePos;
void MainGLWidget::handleCameraRotate(QMouseEvent* evt) {
    if (!isMouseRightDragging) return;
    
    camera.processRotation(evt->pos().x() - lastMousePos.x(), evt->pos().y() - lastMousePos.y());
    lastMousePos = evt->pos();

    // QCursor::setPos(lastMousePos);
}

bool isMouseDragging = false;
std::optional<std::weak_ptr<Part>> draggingObject;
void MainGLWidget::handleObjectDrag(QMouseEvent* evt) {
    if (!isMouseDragging) return;

    QPoint position = evt->pos();

    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));
    std::optional<const RaycastResult> rayHit = castRayNearest(camera.cameraPos, pointDir, 50000, [](std::shared_ptr<Part> part) {
        return (part == draggingObject->lock()) ? FilterResult::PASS : FilterResult::TARGET;
    });
    
    if (!rayHit) return;
    Data::Vector3 vec = rayHit->worldPoint;
    vec = vec + Data::Vector3(rpToGlm(rayHit->worldNormal) * draggingObject->lock()->scale / 2.f);
    draggingObject->lock()->cframe = draggingObject->lock()->cframe.Rotation() + vec;
    syncPartPhysics(draggingObject->lock());
}

void MainGLWidget::handleCursorChange(QMouseEvent* evt) {
    QPoint position = evt->pos();

    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));
    std::optional<const RaycastResult> rayHit = castRayNearest(camera.cameraPos, pointDir, 50000);
    setCursor((rayHit && partFromBody(rayHit->body)->name != "Baseplate") ? Qt::OpenHandCursor : Qt::ArrowCursor);
}

void MainGLWidget::mouseMoveEvent(QMouseEvent* evt) {
    handleCameraRotate(evt);
    handleObjectDrag(evt);
    handleCursorChange(evt);
}

void MainGLWidget::mousePressEvent(QMouseEvent* evt) {
    switch(evt->button()) {
    // Camera drag
    case Qt::RightButton: {
        lastMousePos = evt->pos();
        isMouseRightDragging = true;
        return;
    // Clicking on objects
    } case Qt::LeftButton: {
        QPoint position = evt->pos();

        glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));
        std::optional<const RaycastResult> rayHit = castRayNearest(camera.cameraPos, pointDir, 50000);
        if (!rayHit || !partFromBody(rayHit->body)) return;
        std::shared_ptr<Part> part = partFromBody(rayHit->body);
        if (part->name == "Baseplate") return;
        
        //part.selected = true;
        isMouseDragging = true;
        draggingObject = part;
        setSelection(std::vector<InstanceRefWeak> { part });
        // Disable bit so that we can ignore the part while raycasting
        // part->rigidBody->getCollider(0)->setCollisionCategoryBits(0b10);

        return;
    } default:
        return;
    }
}

void MainGLWidget::mouseReleaseEvent(QMouseEvent* evt) {
    // if (isMouseDragging) draggingObject->lock()->rigidBody->getCollider(0)->setCollisionCategoryBits(0b11);
    isMouseRightDragging = false;
    isMouseDragging = false;
    draggingObject = std::nullopt;
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
        workspace()->AddChild(lastPart = Part::New({
            .position = camera.cameraPos + camera.cameraFront * glm::vec3(3),
            .rotation = glm::vec3(0),
            .scale = glm::vec3(1, 1, 1),
            .color = glm::vec3(1.0f, 0.5f, 0.31f),
        }));
        syncPartPhysics(lastPart);
    }
}

void MainGLWidget::keyReleaseEvent(QKeyEvent* evt) {
    if (evt->key() == Qt::Key_W || evt->key() == Qt::Key_S) moveZ = 0;
    else if (evt->key() == Qt::Key_A || evt->key() == Qt::Key_D) moveX = 0;
}