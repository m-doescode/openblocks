#include <GL/glew.h>
#include <chrono>

#include <QMouseEvent>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <reactphysics3d/collision/RaycastInfo.h>

#include "GLFW/glfw3.h"
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

bool isMouseDragging = false;
QPoint lastMousePos;
void MainGLWidget::mouseMoveEvent(QMouseEvent* evt) {
    // if (!(evt->buttons() & Qt::RightButton)) return;
    if (!isMouseDragging) return;
    
    camera.processRotation(evt->pos().x() - lastMousePos.x(), evt->pos().y() - lastMousePos.y());
    lastMousePos = evt->pos();
    // QCursor::setPos(lastMousePos);
}

class FirstRayHit : public rp::RaycastCallback {
    rp::Body** target;

    virtual rp::decimal notifyRaycastHit(const rp::RaycastInfo& raycastInfo) override {
        if (reinterpret_cast<Part*>(raycastInfo.body->getUserData())->name == "Baseplate") return 1;

        *target = raycastInfo.body;
        return 0;
    }

public:
    FirstRayHit(rp::Body** target) : target(target) {}
};

void MainGLWidget::mousePressEvent(QMouseEvent* evt) {
    switch(evt->button()) {
    // Camera drag
    case Qt::RightButton: {
        lastMousePos = evt->pos();
        isMouseDragging = true;
        return;
    // Clicking on objects
    } case Qt::LeftButton: {
        QPoint position = evt->pos();
        rp::Body* rayHitTarget = NULL;

        // VVV Thank goodness for this person's answer
        // https://stackoverflow.com/a/30005258/16255372

        // glm::vec3 worldPos = camera.cameraPos + glm::vec3(glm::vec4(float(position.x()) / width() - 0.5f, float(position.y()) / height() - 0.5f, 0, 0) * camera.getLookAt());
        glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)width() / (float)height(), 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0), camera.cameraFront, camera.cameraUp);
        glm::mat4 inverseViewport = glm::inverse(projection * view);

        glm::vec2 ndc = glm::vec2(float(position.x()) / width() * 2.f - 1.f, -float(position.y()) / height() * 2.f + 1.f);
        glm::vec4 world = glm::normalize(inverseViewport * glm::vec4(ndc, 1, 1));
        glm::vec3 flat = glm::vec3(world) / world.w; // https://stackoverflow.com/a/68870587/16255372
        
        printf("At: %f; %f; %f\n", world.x, world.y, world.z);

        workspace->AddChild(lastPart = Part::New({
            .position = camera.cameraPos + glm::vec3(world) * 10.f,
            .rotation = glm::vec3(0),
            .scale = glm::vec3(1, 1, 1),
            .material = Material {
                .diffuse = glm::vec3(1.0f, 0.5f, 0.31f),
                .specular = glm::vec3(0.5f, 0.5f, 0.5f),
                .shininess = 32.0f,
            },
            .anchored = true,
        }));
        syncPartPhysics(lastPart);

        castRay(camera.cameraPos, world, 500, new FirstRayHit(&rayHitTarget));
        if (!rayHitTarget) return;
        printf("Hit: %s\n", reinterpret_cast<Part*>(rayHitTarget->getUserData())->name.c_str());
        return;
    } default:
        return;
    }
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