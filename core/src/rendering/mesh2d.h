#pragma once

class Mesh2D {
    unsigned int VBO, VAO;

public:
    int vertexCount;

    Mesh2D(int vertexCount, float* vertices);
    ~Mesh2D();
    void bind();
};