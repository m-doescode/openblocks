#include "torus.h"
#include <cmath>

#include <GL/glew.h>

#define PI 3.1415926535f

struct vec { float x; float y; float z; };

unsigned int torusVAO, torusVBO;
int lastSize = 0;
void initTorus(int tubeSides, int ringSides) {
    // Free existing buffer
    if (torusVAO != 0)
        glDeleteVertexArrays(1, &torusVAO);
    if (torusVBO != 0)
        glDeleteBuffers(1, &torusVBO);

    lastSize = tubeSides * ringSides;

    // Set up buffer
    glGenVertexArrays(1, &torusVAO);
    glBindVertexArray(torusVAO);

    glGenBuffers(1, &torusVBO);    
    glBindBuffer(GL_ARRAY_BUFFER, torusVBO);

    // Dynamic, because we update the vertices often                                   V~~~~~~~~~~~~~~
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * 6 * tubeSides * ringSides, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void genTorusPoint(float* vertex, float outerRadius, float innerRadius, float tubeSides, float ringSides, int tube, int ring) {
    float angle = float(tube) / tubeSides * 2 * PI;
    float xL = innerRadius * cos(angle);
    float yL = innerRadius * sin(angle);

    float outerAngle = float(ring) / ringSides * 2 * PI;
    float x = (outerRadius + xL) * cos(outerAngle);
    float y = (outerRadius + xL) * sin(outerAngle);
    float z = yL;

    vertex[0] = x; vertex[1] = y; vertex[2] = z;
    // TODO: Add normals and tex coords
}

// made by yours truly
void genTorus(float outerRadius, float innerRadius, int tubeSides, int ringSides) {
    // Automatically generate VBO ad-hoc
    if (lastSize != tubeSides * ringSides)
        initTorus(tubeSides, ringSides);
    
    float* vertices = new float[(tubeSides * ringSides * 6) * 8];

    int vi = 0;
    for (int i = 0; i < tubeSides; i++) {
        for (int j = 0; j < ringSides; j++) {
            
            int in = (i+1) % tubeSides;
            int jn = (j+1) % tubeSides;

            genTorusPoint(&vertices[8*vi++], outerRadius, innerRadius, tubeSides, ringSides, i, j);
            genTorusPoint(&vertices[8*vi++], outerRadius, innerRadius, tubeSides, ringSides, in, j);
            genTorusPoint(&vertices[8*vi++], outerRadius, innerRadius, tubeSides, ringSides, in, jn);

            genTorusPoint(&vertices[8*vi++], outerRadius, innerRadius, tubeSides, ringSides, in, jn);
            genTorusPoint(&vertices[8*vi++], outerRadius, innerRadius, tubeSides, ringSides, i, jn);
            genTorusPoint(&vertices[8*vi++], outerRadius, innerRadius, tubeSides, ringSides, i, j);
        }
    }

    // Bind new data
    glBindBuffer(GL_ARRAY_BUFFER, torusVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 8 * 6 * tubeSides * ringSides, vertices); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Draw
    glBindVertexArray(torusVAO);
    glDrawArrays(GL_TRIANGLES, 0, tubeSides * ringSides * 6);
}