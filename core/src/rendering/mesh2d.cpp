#include <glad/gl.h>

#include "mesh2d.h"

Mesh2D::Mesh2D(int vertexCount, float *vertices) : vertexCount(vertexCount) {
    // Generate buffers
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    // Bind vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * (2 + 2) * sizeof(float), vertices, GL_STATIC_DRAW);

    // Bind vertex attributes to VAO
    glBindVertexArray(VAO);

    // Vertex coords
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (2 + 2) * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    // Tex coords
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (2 + 2) * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

Mesh2D::~Mesh2D() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void Mesh2D::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindVertexArray(VAO);
}