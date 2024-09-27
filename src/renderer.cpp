#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include "shader.h"
#include "mesh.h"
#include "defaultmeshes.h"

#include "renderer.h"

Shader *shader = NULL;

void renderInit(GLFWwindow* window) {
    glViewport(0, 0, 500, 500);

    initMeshes();

    // Compile shader
    shader = new Shader("assets/shaders/orange.vs", "assets/shaders/orange.fs");
}

void render(GLFWwindow* window) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use shader
    shader->use();

    CUBE_MESH->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
}