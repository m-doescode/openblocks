#include "torus.h"
#include <cmath>

#include <GL/glew.h>

#define PI 3.1415926535f

struct vec { float x; float y; float z; };

void genTorusPoint(float outerRadius, float innerRadius, float tubeSides, float ringSides, int tube, int ring) {
    float angle = float(tube) / tubeSides * 2 * PI;
    float xL = innerRadius * cos(angle);
    float yL = innerRadius * sin(angle);

    float outerAngle = float(ring) / ringSides * 2 * PI;
    float x = (outerRadius + xL) * cos(outerAngle);
    float y = (outerRadius + xL) * sin(outerAngle);
    float z = yL;

    // return vec { x, y, z };
    glVertex3f(x, y, z);
}

// made by yours truly
void genTorus(float outerRadius, float innerRadius, int tubeSides, int ringSides) {
    glBegin(GL_TRIANGLES);
    
    for (int i = 0; i < tubeSides; i++) {
        float tube = float(i) / tubeSides;
        for (int j = 0; j < ringSides; j++) {
            float ring = float(j) / ringSides;
            
            int in = (i+1) % tubeSides;
            int jn = (j+1) % tubeSides;

            genTorusPoint(outerRadius, innerRadius, tubeSides, ringSides, i, j);
            genTorusPoint(outerRadius, innerRadius, tubeSides, ringSides, in, j);
            genTorusPoint(outerRadius, innerRadius, tubeSides, ringSides, in, jn);

            genTorusPoint(outerRadius, innerRadius, tubeSides, ringSides, in, jn);
            genTorusPoint(outerRadius, innerRadius, tubeSides, ringSides, i, jn);
            genTorusPoint(outerRadius, innerRadius, tubeSides, ringSides, i, j);
        }
    }

    glEnd();
}