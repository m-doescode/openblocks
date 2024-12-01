#include "GL/glew.h"

#include "part.h"
#include "rendering/renderer.h"
#include "physics/simulation.h"
#include "camera.h"

#include "common.h"

#include "mainglwidget.h"

MainGLWidget::MainGLWidget(QWidget* parent): QOpenGLWidget(parent) {}

void MainGLWidget::initializeGL() {
    // Set up the rendering context, load shaders and other resources, etc.:
    // QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glewInit();
    simulationInit();
    renderInit(NULL);

    // Baseplate
    parts.push_back(Part {
        .position = glm::vec3(0, -5, 0),
        .rotation = glm::vec3(0),
        .scale = glm::vec3(512, 1.2, 512),
        .material = Material {
            .diffuse = glm::vec3(0.388235, 0.372549, 0.384314),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 32.0f,
        },
        .anchored = true,
    });
    syncPartPhysics(parts.back());

    parts.push_back(Part {
        .position = glm::vec3(0),
        .rotation = glm::vec3(0),
        .scale = glm::vec3(4, 1.2, 2),
        .material = Material {
            .diffuse = glm::vec3(0.639216f, 0.635294f, 0.647059f),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 32.0f,
        }
    });
    syncPartPhysics(parts.back());

}

void MainGLWidget::resizeGL(int w, int h) {
    // Update projection matrix and other size related settings:
    // m_projection.setToIdentity();
    // m_projection.perspective(45.0f, w / float(h), 0.01f, 100.0f);
    // ...
}

float lastTime = glfwGetTime();    
void MainGLWidget::paintGL() {
    float deltaTime = glfwGetTime() - lastTime;
    lastTime = glfwGetTime();

    // processInput(NULL);
    physicsStep(deltaTime);
    ::render(NULL);
}
