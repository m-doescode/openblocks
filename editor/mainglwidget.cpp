#include <GL/glew.h>
#include <chrono>

#include <QMouseEvent>
#include <glm/common.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/round.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <numbers>
#include <optional>
#include <reactphysics3d/collision/RaycastInfo.h>
#include <vector>
#include <QSound>

#include "datatypes/cframe.h"
#include "datatypes/vector.h"
#include "editorcommon.h"
#include "logger.h"
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
#include "math_helper.h"
#include "rendering/surface.h"

static Data::CFrame XYZToZXY(glm::vec3(0, 0, 0), -glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

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

glm::vec2 firstPoint;
glm::vec2 secondPoint;

extern std::optional<std::weak_ptr<Part>> draggingObject;
extern std::optional<HandleFace> draggingHandle;
extern Shader* shader;
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


static Data::Vector3 orthoVecs[6] {
    {1, 0, 0},
    {-1, 0, 0},
    {0, 1, 0},
    {0, -1, 0},
    {0, 0, 1},
    {0, 0, -1},
};

// Snaps CFrame to the neareest 90 degree angle
// https://gamedev.stackexchange.com/a/183342
Data::CFrame snapCFrame(Data::CFrame frame) {
    Data::Vector3 closestVec1{0, 0, 0};
    float closest1 = 0.f;

    // Primary vector
    for (Data::Vector3 vec : orthoVecs) {
        float closeness = glm::dot((glm::vec3)frame.LookVector(), (glm::vec3)vec);
        if (closeness > closest1) {
            closest1 = closeness;
            closestVec1 = vec;
        }
    }

    Data::Vector3 closestVec2{0, 0, 0};
    float closest2 = 0.f;

    // Second vector
    for (Data::Vector3 vec : orthoVecs) {
        // Guard against accidental linear dependency
        if (vec == closestVec1) continue;

        float closeness = glm::dot((glm::vec3)frame.UpVector(), (glm::vec3)vec);
        if (closeness > closest2) {
            closest2 = closeness;
            closestVec2 = vec;
        }
    }

    // Data::Vector3 thirdVec = closestVec1.Cross(closestVec2);
    return Data::CFrame(frame.Position(), frame.Position() + closestVec1, closestVec2);
}

bool isMouseDragging = false;
std::optional<std::weak_ptr<Part>> draggingObject;
std::optional<HandleFace> draggingHandle;
void MainGLWidget::handleObjectDrag(QMouseEvent* evt) {
    if (!isMouseDragging || !draggingObject || mainWindow()->selectedTool >= TOOL_SMOOTH) return;

    QPoint position = evt->pos();

    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));
    std::optional<const RaycastResult> rayHit = castRayNearest(camera.cameraPos, pointDir, 50000, [](std::shared_ptr<Part> part) {
        return (part == draggingObject->lock()) ? FilterResult::PASS : FilterResult::TARGET;
    });
    
    if (!rayHit) return;

    glm::vec3 partSize = partFromBody(rayHit->body)->size;
    Data::Vector3 vec = rayHit->worldPoint;
    Data::CFrame targetFrame = partFromBody(rayHit->body)->cframe;
    Data::Vector3 surfaceNormal = targetFrame.Inverse().Rotation() * rayHit->worldNormal;
    // The part being dragged's frame local to the hit target's frame, but without its position component
    // To find a world vector local to the new frame, use newFrame, not localFrame, as localFrame is localFrame is local to targetFrame in itself
    Data::CFrame localFrame = (targetFrame.Inverse() * (draggingObject->lock()->cframe.Rotation() + vec));

    // Snap axis
    localFrame = snapCFrame(localFrame);

    // Snap to studs
    Data::Vector3 inverseSurfaceNormal = Data::Vector3::ONE - surfaceNormal.Abs();
    glm::vec3 inverseNormalPartSize = (Data::Vector3)(partSize + 1.f) * inverseSurfaceNormal / 2.f;
    if (snappingFactor() > 0)
        localFrame = localFrame.Rotation() + glm::round(glm::vec3(localFrame.Position() * inverseSurfaceNormal - inverseNormalPartSize) / snappingFactor()) * snappingFactor() + inverseNormalPartSize
                                            + localFrame.Position() * surfaceNormal.Abs();

    Data::CFrame newFrame = targetFrame * localFrame;

    // Unsink the object
    // Get the normal of the surface relative to the part's frame, and get the size along that vector
    Data::Vector3 unsinkOffset = newFrame.Rotation() * ((newFrame.Rotation().Inverse() * rayHit->worldNormal) * draggingObject->lock()->size / 2);

    draggingObject->lock()->cframe = newFrame + unsinkOffset;

    syncPartPhysics(draggingObject->lock());
}

inline glm::vec3 vec3fy(glm::vec4 vec) {
    return vec / vec.w;
}

// Taken from Godot's implementation of moving handles (godot/editor/plugins/gizmos/gizmo_3d_helper.cpp)
void MainGLWidget::handleLinearTransform(QMouseEvent* evt) {
    if (!isMouseDragging || !draggingHandle || !editorToolHandles->adornee || !editorToolHandles->active) return;

    QPoint position = evt->pos();

    auto part = editorToolHandles->adornee->lock();

    // This was actually quite a difficult problem to solve, managing to get the handle to go underneath the cursor

    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));
    pointDir = glm::normalize(pointDir);

    Data::CFrame handleCFrame = editorToolHandles->GetCFrameOfHandle(draggingHandle.value());
    
    // Current frame. Identity frame if worldMode == true, selected object's frame if worldMode == false
    Data::CFrame frame = editorToolHandles->worldMode ? Data::CFrame::IDENTITY + part->position() : part->cframe.Rotation();

    // Segment from axis stretching -4096 to +4096 rel to handle's position 
    glm::vec3 axisSegment0 = handleCFrame.Position() + (-handleCFrame.LookVector() * 4096.0f);
    glm::vec3 axisSegment1 = handleCFrame.Position() + (-handleCFrame.LookVector() * -4096.0f);

    // Segment from camera stretching 4096 forward
    glm::vec3 mouseSegment0 = camera.cameraPos;
    glm::vec3 mouseSegment1 = camera.cameraPos + pointDir * 4096.0f;

    // Closest point on the axis segment between the two segments
    glm::vec3 handlePoint, rb;
    get_closest_points_between_segments(axisSegment0, axisSegment1, mouseSegment0, mouseSegment1, handlePoint, rb);

    // Find new part position
    glm::vec3 centerPoint = editorToolHandles->PartCFrameFromHandlePos(draggingHandle.value(), handlePoint).Position();

    // Apply snapping in the current frame
    glm::vec3 diff = centerPoint - (glm::vec3)editorToolHandles->adornee->lock()->position();
    // printf("\n=======\nPre-snap: (%f, %f, %f)\n", diff.x, diff.y, diff.z);
    if (snappingFactor()) diff = frame.Rotation() * (glm::round(glm::vec3(frame.Inverse().Rotation() * diff) / snappingFactor()) * snappingFactor());
    // printf("Post-snap: (%f, %f, %f)\n", diff.x, diff.y, diff.z);

    switch (mainWindow()->selectedTool) {
        case TOOL_MOVE: {
            // Add difference
            editorToolHandles->adornee->lock()->cframe = editorToolHandles->adornee->lock()->cframe + diff;
        } break;
        
        case TOOL_SCALE: {
            // Find local difference
            glm::vec3 localDiff = frame.Inverse() * diff;
            // Find outwarwd difference
            localDiff = localDiff * glm::sign(draggingHandle->normal);

            // Minimum size of 0.01f
            localDiff = glm::max(part->size + localDiff, 0.01f) - part->size;
            diff = frame * (localDiff * glm::sign(draggingHandle->normal));

            // Add local difference to size
            part->size += localDiff;

            // If ctrl is not pressed, offset the part by half the size difference to keep the other bound where it was originally
            if (!(evt->modifiers() & Qt::ControlModifier)) 
                part->cframe = part->cframe + diff * 0.5f;
        } break;

        default:
        Logger::error("Invalid tool was set to be handled by handleLinearTransform\n");
    }

    syncPartPhysics(std::dynamic_pointer_cast<Part>(editorToolHandles->adornee->lock()));
}

// Also implemented based on Godot: [c7ea8614](godot/editor/plugins/canvas_item_editor_plugin.cpp#L1490)
glm::vec2 startPoint;
Data::CFrame initialFrame = Data::CFrame::IDENTITY;
void MainGLWidget::handleRotationalTransform(QMouseEvent* evt) {
    if (!isMouseDragging || !draggingHandle || !editorToolHandles->adornee || !editorToolHandles->active) return;

    glm::vec2 destPoint = glm::vec2(evt->pos().x(), evt->pos().y());
    auto part = editorToolHandles->adornee->lock();

    // Calculate part pos as screen point
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)width() / (float)height(), 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();

    // The rotated part's origin projected onto the screen
    glm::vec4 partCenterRaw = projection * view * glm::vec4((glm::vec3)initialFrame.Position(), 1.f);
    partCenterRaw /= partCenterRaw.w;
    glm::vec2 partCenter = glm::vec2(partCenterRaw.x*0.5f + 0.5f, 1-(partCenterRaw.y*0.5f+0.5f));
    partCenter *= glm::vec2(width(), height());

    // https://wumbo.net/formulas/angle-between-two-vectors-2d/
    glm::vec2 initVec = glm::normalize(startPoint - (glm::vec2)partCenter);
    glm::vec2 destVec = glm::normalize(destPoint - (glm::vec2)partCenter);
    float angle = atan2f(initVec.x * destVec.y - initVec.y * destVec.x, initVec.x * destVec.x + initVec.y * destVec.y);
    
    // Snap the angle
    if (snappingFactor() > 0)
        angle = roundf(angle * 4 / std::numbers::pi / snappingFactor()) / 4 * std::numbers::pi * snappingFactor();

    // Checks if the rotation axis is facing towards, or away from the camera
    // If it pointing away from the camera, then we need to invert the angle change
    glm::vec4 rotationAxis = projection * view * glm::vec4((glm::vec3)(initialFrame * glm::abs(draggingHandle->normal)), 1.f);
    rotationAxis /= rotationAxis.w;
    glm::vec4 signVec = glm::normalize(rotationAxis - partCenterRaw);
    float sign = -glm::sign(signVec.z);

    glm::vec3 angles = glm::abs(draggingHandle->normal) * sign * glm::vec3(angle);

    part->cframe = initialFrame * Data::CFrame::FromEulerAnglesXYZ(-angles);

    syncPartPhysics(std::dynamic_pointer_cast<Part>(editorToolHandles->adornee->lock()));
}

std::optional<HandleFace> MainGLWidget::raycastHandle(glm::vec3 pointDir) {
    if (!editorToolHandles->adornee.has_value() || !editorToolHandles->active) return std::nullopt;
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
    handleCursorChange(evt);

    switch (mainWindow()->selectedTool) {
    case TOOL_MOVE:
    case TOOL_SCALE:
        handleLinearTransform(evt);
        break;
    case TOOL_ROTATE:
        handleRotationalTransform(evt);
        break;
    default:
        break;
    }
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
            startPoint = glm::vec2(evt->pos().x(), evt->pos().y());
            initialFrame = editorToolHandles->adornee->lock()->cframe;
            isMouseDragging = true;
            draggingHandle = handle;
            return;
        }

        // raycast part
        std::optional<const RaycastResult> rayHit = castRayNearest(camera.cameraPos, pointDir, 50000);
        if (!rayHit || !partFromBody(rayHit->body)) return;
        std::shared_ptr<Part> part = partFromBody(rayHit->body);
        if (part->name == "Baseplate") return;
        
        // Handle surface tool
        if (mainWindow()->selectedTool >= TOOL_SMOOTH) {
            Data::Vector3 localNormal = part->cframe.Inverse().Rotation() * rayHit->worldNormal;
            NormalId face = faceFromNormal(localNormal);
            SurfaceType surface = SurfaceType(mainWindow()->selectedTool - TOOL_SMOOTH);

            switch (face) {
                case Right: part->rightSurface = surface; break;
                case Top: part->topSurface = surface; break;
                case Back: part->backSurface = surface; break;
                case Left: part->leftSurface = surface; break;
                case Bottom: part->bottomSurface = surface; break;
                case Front: part->frontSurface = surface; break;
                default: return;
            }

            if (QFile::exists("./assets/excluded/electronicpingshort.wav"))
                QSound::play("./assets/excluded/electronicpingshort.wav");

            return;
        }

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
        gWorkspace()->AddChild(lastPart = Part::New({
            .position = camera.cameraPos + camera.cameraFront * glm::vec3(3),
            .rotation = glm::vec3(0),
            .size = glm::vec3(1, 1, 1),
            .color = glm::vec3(1.0f, 0.5f, 0.31f),
        }));
        syncPartPhysics(lastPart);
    }

    if (evt->key() == Qt::Key_U)
        Logger::info("info message");
    if (evt->key() == Qt::Key_I)
        Logger::warning("warning message");
    if (evt->key() == Qt::Key_O)
        Logger::error("error message");
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