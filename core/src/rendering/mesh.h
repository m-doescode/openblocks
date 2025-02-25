#pragma once

class Mesh {
    unsigned int VBO, VAO;

public:
    int vertexCount;

    Mesh(int vertexCount, float* vertices);
    ~Mesh();
    void bind();
};