#pragma once

class Mesh {
    unsigned int VBO, VAO;

public:
    Mesh(int vertexCount, float* vertices);
    ~Mesh();
    void bind();
};