#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cstdio>
#include <glm/ext.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <vector>

#include "datatypes/cframe.h"
#include "physics/util.h"
#include "shader.h"
#include "mesh.h"
#include "defaultmeshes.h"
#include "../camera.h"
#include "../common.h"
#include "../objects/part.h"
#include "skybox.h"
#include "surface.h"
#include "texture3d.h"

#include "renderer.h"

Shader* shader = NULL;
Shader* skyboxShader = NULL;
Shader* handleShader = NULL;
Shader* identityShader = NULL;
extern Camera camera;
Skybox* skyboxTexture = NULL;
Texture3D* studsTexture = NULL;

static int viewportWidth, viewportHeight;

void renderInit(GLFWwindow* window, int width, int height) {
    viewportWidth = width, viewportHeight = height;
    glViewport(0, 0, width, height);

    initMeshes();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    skyboxTexture = new Skybox({
        "assets/textures/skybox/null_plainsky512_lf.jpg",
        "assets/textures/skybox/null_plainsky512_rt.jpg",
        "assets/textures/skybox/null_plainsky512_up.jpg",
        "assets/textures/skybox/null_plainsky512_dn.jpg",
        "assets/textures/skybox/null_plainsky512_ft.jpg",
        "assets/textures/skybox/null_plainsky512_bk.jpg",
        }, GL_RGB);

    studsTexture = new Texture3D("assets/textures/studs.png", 128, 128, 6, GL_RGBA);

    // Compile shaders
    shader = new Shader("assets/shaders/phong.vs", "assets/shaders/phong.fs");
    skyboxShader = new Shader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    handleShader = new Shader("assets/shaders/handle.vs", "assets/shaders/handle.fs");
    identityShader = new Shader("assets/shaders/identity.vs", "assets/shaders/identity.fs");
}

void renderParts() {
    glDepthMask(GL_TRUE);

    // Use shader
    shader->use();
    // shader->set("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
    // shader->set("lightColor",  glm::vec3(1.0f, 1.0f, 1.0f));

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    shader->set("projection", projection);
    shader->set("view", view);
    // shader->set("material", Material {
    //     // .ambient = glm::vec3(1.0f, 0.5f, 0.31f),
    //     .diffuse = glm::vec3(0.639216f, 0.635294f, 0.647059f),
    //     .specular = glm::vec3(0.5f, 0.5f, 0.5f),
    //     .shininess = 16.0f,
    // });
    shader->set("sunLight", DirLight {
        .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        .ambient = glm::vec3(0.2f, 0.2f, 0.2f),
        .diffuse = glm::vec3(0.5f, 0.5f, 0.5f),
        .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    });
    shader->set("numPointLights", 0);
    // shader->set("pointLights[0]", PointLight {
    //     .position = lightPos,
    //     .ambient = glm::vec3(0.4f, 0.4f, 0.4f),
    //     .diffuse = glm::vec3(1.0f, 1.0f, 1.0f),
    //     .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    //     .constant = 1.0,
    //     .linear = 0.9,
    //     .quadratic = 0.32,
    // });
    studsTexture->activate(0);
    shader->set("studs", 0);
    // shader->set("surfaces[1]", SurfaceStuds);
    shader->set("surfaces[1]", SurfaceStuds);
    shader->set("surfaces[4]", SurfaceInlets);

    // Pre-calculate the normal matrix for the shader

    // Pass in the camera position
    shader->set("viewPos", camera.cameraPos);

    // TODO: Same as todo in src/physics/simulation.cpp
    for (InstanceRef inst : workspace()->GetChildren()) {
        if (inst->GetClass()->className != "Part") continue;
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst);
        glm::mat4 model = part->cframe;
        if (inst->name == "camera") model = camera.getLookAt();
        model = glm::scale(model, part->size);
        shader->set("model", model);
        shader->set("material", Material {
            .diffuse = part->color,
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 16.0f,
        });
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        shader->set("normalMatrix", normalMatrix);
        shader->set("texScale", part->size);

        CUBE_MESH->bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void renderSkyBox() {
    glDepthMask(GL_FALSE);

    skyboxShader->use();

    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    // Remove translation component of view, making us always at (0, 0, 0)
    glm::mat4 view = glm::mat4(glm::mat3(camera.getLookAt()));

    skyboxShader->set("projection", projection);
    skyboxShader->set("view", view);

    skyboxShader->set("uTexture", 0);

    CUBE_MESH->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void renderHandles() {
    if (!editorToolHandles->adornee.has_value() || !editorToolHandles->active) return;

    glDepthMask(GL_TRUE);
    
    // Use shader
    handleShader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    handleShader->set("projection", projection);
    handleShader->set("view", view);
    handleShader->set("sunLight", DirLight {
        .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        .ambient = glm::vec3(0.2f, 0.2f, 0.2f),
        .diffuse = glm::vec3(0.5f, 0.5f, 0.5f),
        .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    });
    handleShader->set("numPointLights", 0);

    // Pass in the camera position
    handleShader->set("viewPos", camera.cameraPos);

    for (auto face : HandleFace::Faces) {
        glm::mat4 model = editorToolHandles->GetCFrameOfHandle(face);
        model = glm::scale(model, (glm::vec3)editorToolHandles->HandleSize(face));
        handleShader->set("model", model);
        handleShader->set("material", Material {
            .diffuse = glm::abs(face.normal),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 16.0f,
        });
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        handleShader->set("normalMatrix", normalMatrix);

        if (editorToolHandles->handlesType == HandlesType::MoveHandles) {
            ARROW_MESH->bind();
            glDrawArrays(GL_TRIANGLES, 0, ARROW_MESH->vertexCount);
        } else {
            SPHERE_MESH->bind();
            glDrawArrays(GL_TRIANGLES, 0, SPHERE_MESH->vertexCount);
        }
    }

    // 2d square overlay
    identityShader->use();
    identityShader->set("aColor", glm::vec3(0.f, 1.f, 1.f));
    
    for (auto face : HandleFace::Faces) {
        Data::CFrame cframe = editorToolHandles->GetCFrameOfHandle(face);
        glm::vec4 screenPos = projection * view * glm::vec4((glm::vec3)cframe.Position(), 1.0f);
        glm::vec3 ndcCoords = screenPos / screenPos.w;

        float rad = 5;
        float xRad = rad * 1/viewportWidth;
        float yRad = rad * 1/viewportHeight;

        glBegin(GL_QUADS);
            glVertex3f(ndcCoords.x - xRad, ndcCoords.y - yRad, 0);
            glVertex3f(ndcCoords.x + xRad, ndcCoords.y - yRad, 0);
            glVertex3f(ndcCoords.x + xRad, ndcCoords.y + yRad, 0);
            glVertex3f(ndcCoords.x - xRad, ndcCoords.y + yRad, 0);
        glEnd();
    }
}

void render(GLFWwindow* window) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSkyBox();
    renderHandles();
    renderParts();
}

void setViewport(int width, int height) {
    viewportWidth = width, viewportHeight = height;
}