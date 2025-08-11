#include <glad/gl.h>
#include <glm/common.hpp>
#include <glm/vector_relational.hpp>
#include <memory>
#include <miniaudio.h>
#include <qnamespace.h>
#include <string>
#include "./ui_mainwindow.h"
#include "mainglwidget.h"
#include "datatypes/vector.h"
#include "enum/surface.h"
#include "handles.h"
#include "logger.h"
#include "mainwindow.h"
#include "common.h"
#include "math_helper.h"
#include "objects/base/instance.h"
#include "objects/pvinstance.h"
#include "objects/service/selection.h"
#include "partassembly.h"
#include "physics/util.h"
#include "rendering/renderer.h"
#include "rendering/shader.h"
#include "datatypes/variant.h"
#include "undohistory.h"

#define PI 3.14159
#define M_mainWindow dynamic_cast<MainWindow*>(window())

static CFrame XYZToZXY(glm::vec3(0, 0, 0), -glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

MainGLWidget::MainGLWidget(QWidget* parent): QOpenGLWidget(parent), contextMenu(this) {
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setMouseTracking(true);
}

void MainGLWidget::initializeGL() {
    int version = gladLoaderLoadGL();
    if (version == 0) {
        Logger::fatalError("Failed to initialize OpenGL context");
        panic();
    } else {
        Logger::debugf("Initialized GL context version %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    }
    renderInit(width(), height());
}

extern ma_engine miniaudio;
inline void playSound(QString path) {
    ma_engine_stop(&miniaudio);
    ma_engine_play_sound(&miniaudio, path.toStdString().c_str(), NULL);
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

extern std::weak_ptr<BasePart> draggingObject;
extern std::optional<HandleFace> draggingHandle;
extern Shader* shader;
void MainGLWidget::paintGL() {
    ::render();
}

bool isMouseRightDragging = false;
QPoint lastMousePos;
void MainGLWidget::handleCameraRotate(QMouseEvent* evt) {
    if (!isMouseRightDragging) return;
    
    camera.processRotation(evt->pos().x() - lastMousePos.x(), evt->pos().y() - lastMousePos.y());
    lastMousePos = evt->pos();

    // QCursor::setPos(lastMousePos);
}


static Vector3 orthoVecs[6] {
    {1, 0, 0},
    {-1, 0, 0},
    {0, 1, 0},
    {0, -1, 0},
    {0, 0, 1},
    {0, 0, -1},
};

// Snaps CFrame to the neareest 90 degree angle
// https://gamedev.stackexchange.com/a/183342
CFrame snapCFrame(CFrame frame) {
    Vector3 closestVec1{0, 0, 0};
    float closest1 = 0.f;

    // Primary vector
    for (Vector3 vec : orthoVecs) {
        float closeness = glm::dot((glm::vec3)frame.LookVector(), (glm::vec3)vec);
        if (closeness > closest1) {
            closest1 = closeness;
            closestVec1 = vec;
        }
    }

    Vector3 closestVec2{0, 0, 0};
    float closest2 = 0.f;

    // Second vector
    for (Vector3 vec : orthoVecs) {
        // Guard against accidental linear dependency
        if (vec == closestVec1) continue;

        float closeness = glm::dot((glm::vec3)frame.UpVector(), (glm::vec3)vec);
        if (closeness > closest2) {
            closest2 = closeness;
            closestVec2 = vec;
        }
    }

    // Vector3 thirdVec = closestVec1.Cross(closestVec2);
    return CFrame(frame.Position(), frame.Position() + closestVec1, closestVec2);
}

std::vector<PartTransformState> initialTransforms;

bool tryMouseContextMenu = false;
bool isMouseDragging = false;
std::weak_ptr<BasePart> draggingObject;
std::optional<HandleFace> draggingHandle;
Vector3 initialHitPos;
Vector3 initialHitNormal;
CFrame initialFrame;
PartAssembly initialAssembly({});
void MainGLWidget::handleObjectDrag(QMouseEvent* evt) {
    if (!isMouseDragging || draggingObject.expired() || mainWindow()->selectedTool >= TOOL_SMOOTH) return;

    QPoint position = evt->pos();

    initialAssembly.SetCollisionsEnabled(false);
    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));
    std::optional<const RaycastResult> rayHit = gWorkspace()->CastRayNearest(camera.cameraPos, pointDir, 50000);
    initialAssembly.SetCollisionsEnabled(true);
    
    if (!rayHit) return;

    CFrame targetFrame = partFromBody(rayHit->body)->cframe;
    Vector3 surfaceNormal = targetFrame.Inverse().Rotation() * rayHit->worldNormal;
    Vector3 inverseSurfaceNormal = Vector3::ONE - surfaceNormal.Abs();
    glm::vec3 partSize = partFromBody(rayHit->body)->size;
    Vector3 tFormedHitPos = targetFrame * ((targetFrame.Inverse() * initialHitPos) * inverseSurfaceNormal);
    Vector3 tFormedInitialPos = targetFrame * ((targetFrame.Inverse() * initialFrame.Position()) * inverseSurfaceNormal);
    Vector3 vec = rayHit->worldPoint + (tFormedInitialPos - tFormedHitPos);
    // The part being dragged's frame local to the hit target's frame, but without its position component
    // To find a world vector local to the new frame, use newFrame, not localFrame, as localFrame is localFrame is local to targetFrame in itself
    CFrame localFrame = (targetFrame.Inverse() * (initialAssembly.assemblyOrigin().Rotation() + vec));

    // Snap axis
    localFrame = snapCFrame(localFrame);

    // Snap to studs
    Vector3 draggingPartSize = initialAssembly.size();
    glm::vec3 inverseNormalPartSize = (Vector3)(partSize - glm::vec3(localFrame.Rotation() * draggingPartSize)) * inverseSurfaceNormal / 2.f;
    if (snappingFactor() > 0)
        localFrame = localFrame.Rotation() + glm::round(glm::vec3(localFrame.Position() * inverseSurfaceNormal - inverseNormalPartSize) / snappingFactor()) * snappingFactor() + inverseNormalPartSize
                                           + localFrame.Position() * surfaceNormal.Abs();

    CFrame newFrame = targetFrame * localFrame;

    // Unsink the object
    // Get the normal of the surface relative to the part's frame, and get the size along that vector
    Vector3 unsinkOffset = newFrame.Rotation() * ((newFrame.Rotation().Inverse() * rayHit->worldNormal) * initialAssembly.size() / 2);

    initialAssembly.SetOrigin(newFrame + unsinkOffset);
}

inline glm::vec3 vec3fy(glm::vec4 vec) {
    return vec / vec.w;
}

// Taken from Godot's implementation of moving handles (godot/editor/plugins/gizmos/gizmo_3d_helper.cpp)
Vector3 dragStartHandleOffset;
void MainGLWidget::handleLinearTransform(QMouseEvent* evt) {
    if (!isMouseDragging || !draggingHandle|| !editorToolHandles.active) return;

    QPoint position = evt->pos();

    // auto part = getHandleAdornee();

    // This was actually quite a difficult problem to solve, managing to get the handle to go underneath the cursor

    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));
    pointDir = glm::normalize(pointDir);

    // We use lastDragStartPos instead to consider the mouse's actual position, rather than the center
    // of the handle. That way, transformations are "smoother" and do not jump the first movement
    CFrame handleCFrame = getHandleCFrame(draggingHandle.value()) + dragStartHandleOffset;
    
    // Current frame. Identity frame if worldMode == true, selected object's frame if worldMode == false
    CFrame frame = editorToolHandles.worldMode ? CFrame::IDENTITY + initialAssembly.assemblyOrigin().Position() : initialAssembly.assemblyOrigin().Rotation();

    // Segment from axis stretching -4096 to +4096 rel to handle's position 
    glm::vec3 axisSegment0 = handleCFrame.Position() + (-handleCFrame.LookVector() * 4096.0f);
    glm::vec3 axisSegment1 = handleCFrame.Position() + (-handleCFrame.LookVector() * -4096.0f);

    // Segment from camera stretching 4096 forward
    glm::vec3 mouseSegment0 = camera.cameraPos;
    glm::vec3 mouseSegment1 = camera.cameraPos + pointDir * 4096.0f;

    // Closest point on the axis segment between the two segments
    glm::vec3 handlePoint, rb;
    get_closest_points_between_segments(axisSegment0, axisSegment1, mouseSegment0, mouseSegment1, handlePoint, rb);

    // We transform the handlePoint to the handle's cframe, and get it's length (Z)
    float diff = (handleCFrame.Inverse() * handlePoint).Z();
    // Vector3 absDiff = ((Vector3)handlePoint - handleCFrame.Position()); // Commented out because it is functionally identical to the below
    Vector3 absDiff = handleCFrame.Rotation() * Vector3(0, 0, diff);

    // Apply snapping
    if (snappingFactor() > 0) {
        diff = round(diff / snappingFactor()) * snappingFactor();
        absDiff = handleCFrame.Rotation() * Vector3(0, 0, diff);
    }

    PartAssembly selectionAssembly = PartAssembly::FromSelection(gDataModel->GetService<Selection>());

    if (editorToolHandles.handlesType == MoveHandles) {
        selectionAssembly.TransformBy(CFrame() + absDiff);
    } else if (editorToolHandles.handlesType == ScaleHandles) {
        Vector3 oldSize = selectionAssembly.size();
        if (evt->modifiers() & Qt::AltModifier) {
            // If size gets too small, don't
            if (glm::any(glm::lessThan(glm::vec3(selectionAssembly.size() + abs(draggingHandle->normal) * diff * 2.f), glm::vec3(0.001f))))
                return;

            selectionAssembly.Scale(selectionAssembly.size() + abs(draggingHandle->normal) * diff * 2.f, diff > 0);
        } else {
            // If size gets too small, don't
            if (glm::any(glm::lessThan(glm::vec3(selectionAssembly.size() + abs(draggingHandle->normal) * diff), glm::vec3(0.001f))))
                return;

            // This causes the velocity to be reset even though it shouldn't, but it's not a huge deal, so whatevs.
            selectionAssembly.TransformBy(CFrame() + absDiff * 0.5f);
            selectionAssembly.Scale(selectionAssembly.size() + abs(draggingHandle->normal) * diff, diff > 0);
        }

        if (snappingFactor() > 0 && oldSize != selectionAssembly.size() && mainWindow()->editSoundEffects && QFile::exists("./assets/excluded/switch.wav"))
            playSound("./assets/excluded/switch.wav");
    }
}

void MainGLWidget::startLinearTransform(QMouseEvent* evt) {
    if (!editorToolHandles.active) return;

    QPoint position = evt->pos();

    // This was actually quite a difficult problem to solve, managing to get the handle to go underneath the cursor

    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));
    pointDir = glm::normalize(pointDir);

    CFrame handleCFrame = getHandleCFrame(draggingHandle.value());

    // Segment from axis stretching -4096 to +4096 rel to handle's position 
    glm::vec3 axisSegment0 = handleCFrame.Position() + (-handleCFrame.LookVector() * 4096.0f);
    glm::vec3 axisSegment1 = handleCFrame.Position() + (-handleCFrame.LookVector() * -4096.0f);

    // Segment from camera stretching 4096 forward
    glm::vec3 mouseSegment0 = camera.cameraPos;
    glm::vec3 mouseSegment1 = camera.cameraPos + pointDir * 4096.0f;

    // Closest point on the axis segment between the two segments
    glm::vec3 handlePoint, rb;
    get_closest_points_between_segments(axisSegment0, axisSegment1, mouseSegment0, mouseSegment1, handlePoint, rb);

    dragStartHandleOffset = (Vector3)handlePoint - handleCFrame.Position();
}

// Also implemented based on Godot: [c7ea8614](godot/editor/plugins/canvas_item_editor_plugin.cpp#L1490)
glm::vec2 startPoint;
void MainGLWidget::handleRotationalTransform(QMouseEvent* evt) {
    if (!isMouseDragging || !draggingHandle || !editorToolHandles.active) return;

    glm::vec2 destPoint = glm::vec2(evt->pos().x(), evt->pos().y());

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
        angle = roundf(angle * 4 / PI / snappingFactor()) / 4 * PI * snappingFactor();

    glm::vec3 handleNormal = XYZToZXY * glm::abs(draggingHandle->normal);

    // Checks if the rotation axis is facing towards, or away from the camera
    // If it pointing away from the camera, then we need to invert the angle change
    glm::vec4 rotationAxis = projection * view * glm::vec4((glm::vec3)(initialFrame * handleNormal), 1.f);
    rotationAxis /= rotationAxis.w;
    glm::vec4 signVec = glm::normalize(rotationAxis - partCenterRaw);
    float sign = -glm::sign(signVec.z);

    glm::vec3 angles = handleNormal * sign * glm::vec3(angle);

    CFrame newFrame = initialFrame * CFrame::FromEulerAnglesXYZ(-angles);
    initialAssembly.SetOrigin(newFrame);
}

std::optional<HandleFace> MainGLWidget::raycastHandle(glm::vec3 pointDir) {
    if (!editorToolHandles.active) return std::nullopt;
    return ::raycastHandle(rp3d::Ray(glmToRp(camera.cameraPos), glmToRp(glm::normalize(pointDir)) * 50000));
}

void MainGLWidget::handleCursorChange(QMouseEvent* evt) {
    QPoint position = evt->pos();

    glm::vec3 pointDir = camera.getScreenDirection(glm::vec2(position.x(), position.y()), glm::vec2(width(), height()));

    if (raycastHandle(pointDir)) {
        setCursor(Qt::OpenHandCursor);
        return;
    };

    std::optional<const RaycastResult> rayHit = gWorkspace()->CastRayNearest(camera.cameraPos, pointDir, 50000);
    if (rayHit && !partFromBody(rayHit->body)->locked) {
        setCursor(Qt::OpenHandCursor);
        return;
    }

    setCursor(Qt::ArrowCursor);
}

void MainGLWidget::wheelEvent(QWheelEvent* evt) {
    camera.processMovement(evt->angleDelta().y() < 0 ? DIRECTION_BACKWARDS : DIRECTION_FORWARD, 0.25f);

    if (mainWindow()->editSoundEffects && QFile::exists("./assets/excluded/SWITCH3.wav"))
        playSound("./assets/excluded/SWITCH3.wav");
}

void MainGLWidget::mouseMoveEvent(QMouseEvent* evt) {
    tryMouseContextMenu = false;
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
    initialTransforms = {};
    tryMouseContextMenu = evt->button() == Qt::RightButton;
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
            initialAssembly = PartAssembly::FromSelection(gDataModel->GetService<Selection>());
            initialFrame = initialAssembly.assemblyOrigin();
            initialTransforms = PartAssembly::FromSelection(gDataModel->GetService<Selection>()).GetCurrentTransforms();
            isMouseDragging = true;
            draggingHandle = handle;
            startLinearTransform(evt);
            return;
        }

        // raycast part
        std::shared_ptr<Selection> selection = gDataModel->GetService<Selection>();
        std::optional<const RaycastResult> rayHit = gWorkspace()->CastRayNearest(camera.cameraPos, pointDir, 50000);
        if (!rayHit || !partFromBody(rayHit->body)) { selection->Set({}); return; }
        std::shared_ptr<BasePart> part = partFromBody(rayHit->body);
        if (part->locked) { selection->Set({}); return; }

        std::shared_ptr<PVInstance> selObject = part;

        // Traverse to the root model
        if (~evt->modifiers() & Qt::AltModifier) {
            std::optional<std::shared_ptr<Instance>> nextParent = selObject->GetParent();
            while (nextParent.value() && nextParent.value()->IsA("Model")) {
                selObject = std::dynamic_pointer_cast<PVInstance>(nextParent.value()); nextParent = selObject->GetParent();
            }
        }

        initialAssembly = PartAssembly::FromSelection({selObject});
        initialFrame = initialAssembly.assemblyOrigin();
        initialHitPos = rayHit->worldPoint;
        initialHitNormal = rayHit->worldNormal;
        
        // Handle surface tool
        if (mainWindow()->selectedTool >= TOOL_SMOOTH) {
            Vector3 localNormal = part->cframe.Inverse().Rotation() * rayHit->worldNormal;
            NormalId face = faceFromNormal(localNormal);
            SurfaceType surface = SurfaceType(mainWindow()->selectedTool - TOOL_SMOOTH);
            std::string surfacePropertyName = EnumType::NormalId.FromValue(face)->Name() + "Surface";

            // Get old surface and set new surface
            EnumItem newSurface = EnumType::SurfaceType.FromValue((int)surface).value();
            EnumItem oldSurface = part->GetProperty(surfacePropertyName).expect().get<EnumItem>();
            part->SetProperty(surfacePropertyName, newSurface).expect();

            M_mainWindow->undoManager.PushState({UndoStatePropertyChanged { part, surfacePropertyName, oldSurface, newSurface }});

            if (mainWindow()->editSoundEffects && QFile::exists("./assets/excluded/electronicpingshort.wav"))
                playSound("./assets/excluded/electronicpingshort.wav");

            return;
        }

        //part.selected = true;
        isMouseDragging = true;
        draggingObject = part;
        initialTransforms = PartAssembly::FromSelection({part}).GetCurrentTransforms();
        if (evt->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
            auto sel = selection->Get();
            if (std::find(sel.begin(), sel.end(), selObject) == sel.end())
                selection->Add({ selObject });
            else
                selection->Remove({ selObject });
        } else {
            selection->Set({ selObject });
        }
        // Disable bit so that we can ignore the part while raycasting
        // part->rigidBody->getCollider(0)->setCollisionCategoryBits(0b10);

        return;
    } default:
        return;
    }
}

void MainGLWidget::mouseReleaseEvent(QMouseEvent* evt) {
    // if (isMouseDragging) draggingObject.lock()->rigidBody->getCollider(0)->setCollisionCategoryBits(0b11);
    isMouseRightDragging = false;
    isMouseDragging = false;
    draggingObject = {};
    draggingHandle = std::nullopt;

    if (!initialTransforms.empty()) {
        UndoState historyState;

        for (auto t : initialTransforms) {
            historyState.push_back(UndoStatePropertyChanged { t.part, "CFrame", t.cframe, t.part->cframe });
            historyState.push_back(UndoStatePropertyChanged { t.part, "Size", t.size, t.part->size });
            historyState.push_back(UndoStatePropertyChanged { t.part, "Velocity", t.velocity, t.part->velocity });
        }

        M_mainWindow->undoManager.PushState(historyState);
    }

    // Open context menu
    if (tryMouseContextMenu)
        contextMenu.exec(QCursor::pos());
    tryMouseContextMenu = false;
}

void MainGLWidget::buildContextMenu() {
    contextMenu.addAction(M_mainWindow->ui->actionDelete);
    contextMenu.addSeparator();
    contextMenu.addAction(M_mainWindow->ui->actionCopy);
    contextMenu.addAction(M_mainWindow->ui->actionCut);
    contextMenu.addAction(M_mainWindow->ui->actionPaste);
    contextMenu.addAction(M_mainWindow->ui->actionPasteInto);
    contextMenu.addSeparator();
    contextMenu.addAction(M_mainWindow->ui->actionSaveModel);
    contextMenu.addAction(M_mainWindow->ui->actionInsertModel);
}

static int moveZ = 0;
static int moveX = 0;
static int moveYw = 0; // World Y

static std::chrono::time_point lastTime = std::chrono::steady_clock::now();
void MainGLWidget::updateCycle() {
    float deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - lastTime).count();
    lastTime = std::chrono::steady_clock::now();

    if (moveZ)
        camera.processMovement(moveZ == 1 ? DIRECTION_FORWARD : DIRECTION_BACKWARDS, deltaTime);
    if (moveX)
        camera.processMovement(moveX == 1 ? DIRECTION_LEFT : DIRECTION_RIGHT, deltaTime);
    if (moveYw)
        camera.processMovement(moveYw == 1 ? DIRECTION_UP : DIRECTION_DOWN, deltaTime);

}

int partId = 1;
void MainGLWidget::keyPressEvent(QKeyEvent* evt) {
    if (evt->key() == Qt::Key_W) moveZ = 1;
    else if (evt->key() == Qt::Key_S) moveZ = -1;
    
    if (evt->key() == Qt::Key_A) moveX = 1;
    else if (evt->key() == Qt::Key_D) moveX = -1;

    if (evt->key() == Qt::Key_E) moveYw = 1;
    else if (evt->key() == Qt::Key_Q) moveYw = -1;

    if (evt->key() == Qt::Key_F) {
        gWorkspace()->AddChild(lastPart = Part::New({
            .position = camera.cameraPos + camera.cameraFront * glm::vec3(3),
            .rotation = glm::vec3(0),
            .size = glm::vec3(1, 1, 1),
            .color = glm::vec3(1.0f, 0.5f, 0.31f),
        }));
        gWorkspace()->SyncPartPhysics(lastPart);
        lastPart->name = "Part" + std::to_string(partId++);
        M_mainWindow->undoManager.PushState({ UndoStateInstanceCreated { lastPart, gWorkspace() } });
    }

    if (evt->key() == Qt::Key_BracketLeft) {
        static bool debugRenderEnabled;
        debugRenderEnabled = !debugRenderEnabled;
        setDebugRendererEnabled(debugRenderEnabled);
    }
}

void MainGLWidget::keyReleaseEvent(QKeyEvent* evt) {
    if (evt->key() == Qt::Key_W || evt->key() == Qt::Key_S) moveZ = 0;
    else if (evt->key() == Qt::Key_A || evt->key() == Qt::Key_D) moveX = 0;
    else if (evt->key() == Qt::Key_E || evt->key() == Qt::Key_Q) moveYw = 0;
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