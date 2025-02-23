#include <GL/glew.h>
#include <chrono>

#include <QMouseEvent>
#include <glm/common.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/round.hpp>
#include <glm/matrix.hpp>
#include <memory>
#include <optional>
#include <reactphysics3d/collision/RaycastInfo.h>
#include <vector>

#include "datatypes/cframe.h"
#include "editorcommon.h"
#include "mainwindow.h"
#include "objects/handles.h"
#include "physics/util.h"
#include "qcursor.h"
#include "qevent.h"
#include "qnamespace.h"
#include "qwindowdefs.h"
#include "rendering/renderer.h"
#include "physics/simulation.h"
#include "camera.h"

#include "common.h"
#include "rendering/shader.h"

#include "mainglwidget.h"
#include "../core/src/rendering/defaultmeshes.h"
#include "math_helper.h"

MainGLWidget::MainGLWidget(QWidget* parent): QOpenGLWidget(parent) {
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setMouseTracking(true);
}

Shader* identityShader;

void MainGLWidget::initializeGL() {
    glewInit();
    renderInit(NULL, width(), height());

    identityShader = new Shader("assets/shaders/identity.vs", "assets/shaders/identity.fs");
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

glm::vec2 firstPoint;
glm::vec2 secondPoint;

extern std::optional<std::weak_ptr<Part>> draggingObject;
extern std::optional<HandleFace> draggingHandle;
extern Shader* shader;
void MainGLWidget::paintGL() {
    ::render(NULL);

    if (!editorToolHandles->adornee) return;

    for (HandleFace face : HandleFace::Faces) {
        // Ignore negatives (for now)
        if (face.normal != glm::abs(face.normal)) continue;

        glm::vec3 axisNormal = face.normal;
        // // glm::vec3 planeNormal = camera.cameraFront;
        glm::vec3 planeRight = glm::cross(axisNormal, glm::normalize(camera.cameraFront));
        glm::vec3 planeNormal = glm::cross(glm::normalize(planeRight), axisNormal);

        auto a = axisNormal;
        auto b = planeRight;
        auto c = planeNormal;

        glm::mat3 matrix = {
            axisNormal,
            planeRight,
            -planeNormal,
        };

        glm::mat4 model = glm::translate(glm::mat4(1.0f), (glm::vec3)editorToolHandles->adornee->lock()->position()) * glm::mat4(matrix);
        model = glm::scale(model, glm::vec3(5, 5, 0.2));

        shader->set("model", model);
        shader->set("material", Material {
            .diffuse = glm::vec3(0.5, 1.f, 0.5),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 16.0f,
        });
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        shader->set("normalMatrix", normalMatrix);

        CUBE_MESH->bind();
        // glDrawArrays(GL_TRIANGLES, 0, 36);
    }
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
std::optional<HandleFace> draggingHandle;
void MainGLWidget::handleObjectDrag(QMouseEvent* evt) {
    if (!isMouseDragging || !draggingObject) return;

    QPoint position = evt->pos();

    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));
    std::optional<const RaycastResult> rayHit = castRayNearest(camera.cameraPos, pointDir, 50000, [](std::shared_ptr<Part> part) {
        return (part == draggingObject->lock()) ? FilterResult::PASS : FilterResult::TARGET;
    });
    
    if (!rayHit) return;
    Data::Vector3 vec = rayHit->worldPoint;
    vec = vec + Data::Vector3(rpToGlm(rayHit->worldNormal) * draggingObject->lock()->size / 2.f);
    draggingObject->lock()->cframe = draggingObject->lock()->cframe.Rotation() + vec;
    syncPartPhysics(draggingObject->lock());
}

inline glm::vec3 vec3fy(glm::vec4 vec) {
    return vec / vec.w;
}

QPoint lastPoint;
void MainGLWidget::handleHandleDrag(QMouseEvent* evt) {
    QPoint cLastPoint = lastPoint;
    lastPoint = evt->pos();

    if (!isMouseDragging || !draggingHandle || !editorToolHandles->adornee) return;

    QPoint position = evt->pos();

    // This was actually quite a difficult problem to solve, managing to get the handle to go underneath the cursor

    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));
    pointDir = glm::normalize(pointDir);

    Data::CFrame handleCFrame = editorToolHandles->GetCFrameOfHandle(draggingHandle.value());

    // Segment from axis stretching -4096 to +4096 rel to handle's position 
    glm::vec3 axisSegment0 = handleCFrame.Position() + glm::abs(draggingHandle->normal) * 4096.0f;
    glm::vec3 axisSegment1 = handleCFrame.Position() + glm::abs(draggingHandle->normal) * -4096.0f;

    // Segment from camera stretching 4096 forward
    glm::vec3 mouseSegment0 = camera.cameraPos;
    glm::vec3 mouseSegment1 = camera.cameraPos + pointDir * 4096.0f;

    // Closest point on the axis segment between the two segments
    glm::vec3 handlePoint, rb;
    get_closest_points_between_segments(axisSegment0, axisSegment1, mouseSegment0, mouseSegment1, handlePoint, rb);

    if (selectedTool == SelectedTool::MOVE) {
        glm::vec3 newPos = editorToolHandles->PartCFrameFromHandlePos(draggingHandle.value(), handlePoint).Position();
        glm::vec3 oldPos = editorToolHandles->adornee->lock()->cframe.Position();
        glm::vec3 diff = newPos - oldPos;
        
        // Apply snapping
        if (snappingFactor()) diff = glm::floor(diff / snappingFactor()) * snappingFactor();
        newPos = diff + oldPos;

        editorToolHandles->adornee->lock()->cframe = editorToolHandles->adornee->lock()->cframe.Rotation() + newPos;
    } else if (selectedTool == SelectedTool::SCALE) {
        glm::vec3 handlePos = editorToolHandles->PartCFrameFromHandlePos(draggingHandle.value(), handlePoint).Position();
        
        // Find change in handles, and negate difference in sign between axes
        glm::vec3 diff = handlePos - glm::vec3(editorToolHandles->adornee->lock()->position());
        
        // Apply snapping
        if (snappingFactor()) diff = glm::floor(diff / snappingFactor()) * snappingFactor();

        editorToolHandles->adornee->lock()->size += diff * glm::sign(draggingHandle->normal);
        
        // If ctrl is not pressed, also reposition the part such that only the dragged side gets lengthened
        if (!(evt->modifiers() & Qt::ControlModifier)) 
            editorToolHandles->adornee->lock()->cframe = editorToolHandles->adornee->lock()->cframe + (diff / 2.0f);
    }

    syncPartPhysics(std::dynamic_pointer_cast<Part>(editorToolHandles->adornee->lock()));
}

std::optional<HandleFace> MainGLWidget::raycastHandle(glm::vec3 pointDir) {
    if (!editorToolHandles->adornee.has_value()) return std::nullopt;
    return editorToolHandles->RaycastHandle(rp3d::Ray(glmToRp(camera.cameraPos), glmToRp(glm::normalize(pointDir)) * 50000));
}

void MainGLWidget::handleCursorChange(QMouseEvent* evt) {
    QPoint position = evt->pos();

    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));

    if (raycastHandle(pointDir)) {
        setCursor(Qt::OpenHandCursor);
        return;
    };

    std::optional<const RaycastResult> rayHit = castRayNearest(camera.cameraPos, pointDir, 50000);
    if (rayHit && partFromBody(rayHit->body)->name != "Baseplate") {
        setCursor(Qt::OpenHandCursor);
        return;
    }

    setCursor(Qt::ArrowCursor);
}

void MainGLWidget::mouseMoveEvent(QMouseEvent* evt) {
    handleCameraRotate(evt);
    handleObjectDrag(evt);
    handleHandleDrag(evt);
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
        // raycast handles
        auto handle = raycastHandle(pointDir);
        if (handle.has_value()) {
            isMouseDragging = true;
            draggingHandle = handle;
            return;
        }

        // raycast part
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
    draggingHandle = std::nullopt;
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
            .size = glm::vec3(1, 1, 1),
            .color = glm::vec3(1.0f, 0.5f, 0.31f),
        }));
        syncPartPhysics(lastPart);
    }
}

void MainGLWidget::keyReleaseEvent(QKeyEvent* evt) {
    if (evt->key() == Qt::Key_W || evt->key() == Qt::Key_S) moveZ = 0;
    else if (evt->key() == Qt::Key_A || evt->key() == Qt::Key_D) moveX = 0;
}

MainWindow* MainGLWidget::mainWindow() {
    return dynamic_cast<MainWindow*>(window());
}

float MainGLWidget::snappingFactor() {
    switch (mainWindow()->snappingMode) {
        case GridSnappingMode::SNAP_1_STUD: return 1;
        case GridSnappingMode::SNAP_05_STUDS: return 0.5;
        case GridSnappingMode::SNAP_OFF: return 0;
    }
    return 0;
}