#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "shader.h"
#include "mesh.h"
#include "defaultmeshes.h"
#include "../camera.h"

#include "renderer.h"

Shader *shader = NULL;
extern Camera camera;

void renderInit(GLFWwindow* window) {
    glViewport(0, 0, 1200, 900);

    initMeshes();

    glEnable(GL_DEPTH_TEST);

    // Compile shader
    shader = new Shader("assets/shaders/phong.vs", "assets/shaders/phong.fs");
}

void render(GLFWwindow* window) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use shader
    shader->use();
    shader->set("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
    shader->set("lightColor",  glm::vec3(1.0f, 1.0f, 1.0f));

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)1200 / (float)900, 0.1f, 100.0f);
    glm::mat4 view = camera.getLookAt();
    shader->set("projection", projection);
    shader->set("view", view);
    shader->set("material", Material {
        // .ambient = glm::vec3(1.0f, 0.5f, 0.31f),
        .diffuse = glm::vec3(1.0f, 0.5f, 0.31f),
        .specular = glm::vec3(0.5f, 0.5f, 0.5f),
        .shininess = 32.0f,
    });
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

    // Pre-calculate the normal matrix for the shader

    // Pass in the camera position
    shader->set("viewPos", camera.cameraPos);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3( 0.0f, 0.0f, 0.0f));
    // float angle = 20.0f * i;
    // model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    shader->set("model", model);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    shader->set("normalMatrix", normalMatrix);

    CUBE_MESH->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
}