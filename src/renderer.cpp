#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include "shader.h"
#include "mesh.h"

#include "renderer.h"

float verts[] {
    // position         // normals          // tex coords
    0.5f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f,   0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f,   0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f,   0.0f, 0.0f,
};

Shader *shader = NULL;
Mesh *triangleMesh = NULL;

void renderInit(GLFWwindow* window) {
    glViewport(0, 0, 500, 500);

    // Compile shader
    shader = new Shader("assets/shaders/orange.vs", "assets/shaders/orange.fs");

    triangleMesh = new Mesh(3, verts);
}

void render(GLFWwindow* window) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use shader
    shader->use();

    triangleMesh->bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
}