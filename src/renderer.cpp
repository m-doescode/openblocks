#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <memory>
#include "shader.h"

#include "renderer.h"

float verts[] {
    0.5f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
};

unsigned int VAO, VBO;
Shader *shader = NULL;

void renderInit(GLFWwindow* window) {
    glViewport(0, 0, 500, 500);

    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    // Bind vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // Bind vertex attributes to VAO
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Compile shader
    shader = new Shader("assets/shaders/orange.vs", "assets/shaders/orange.fs");
}

void render(GLFWwindow* window) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use shader
    shader->use();

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}