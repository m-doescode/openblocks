#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cstdio>
#include <glm/ext.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <vector>

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

Shader *shader = NULL;
Shader *skyboxShader = NULL;
extern Camera camera;
Skybox* skyboxTexture = NULL;
Texture3D* studsTexture = NULL;

void renderInit(GLFWwindow* window) {
    glViewport(0, 0, 1200, 900);

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

    // Compile shader
    shader = new Shader("assets/shaders/phong.vs", "assets/shaders/phong.fs");
    skyboxShader = new Shader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
}

void renderParts() {
    glDepthMask(GL_TRUE);

    // Use shader
    shader->use();
    // shader->set("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
    // shader->set("lightColor",  glm::vec3(1.0f, 1.0f, 1.0f));

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)1200 / (float)900, 0.1f, 100.0f);
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
    for (InstanceRef inst : workspace->GetChildren()) {
        if (inst->GetClass()->className != "Part") continue;
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, part->position);
        model = model * glm::mat4_cast(part->rotation);
        model = glm::scale(model, part->scale);
        shader->set("model", model);
        shader->set("material", part->material);
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        shader->set("normalMatrix", normalMatrix);
        shader->set("texScale", part->scale);

        CUBE_MESH->bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void renderSkyBox() {
    glDepthMask(GL_FALSE);

    skyboxShader->use();

    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)1200 / (float)900, 0.1f, 100.0f);
    // Remove translation component of view, making us always at (0, 0, 0)
    glm::mat4 view = glm::mat4(glm::mat3(camera.getLookAt()));

    skyboxShader->set("projection", projection);
    skyboxShader->set("view", view);

    skyboxShader->set("uTexture", 0);

    CUBE_MESH->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void render(GLFWwindow* window) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSkyBox();
    renderParts();
}