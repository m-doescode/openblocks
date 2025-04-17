#pragma once

/*
    Generates a torus made of circles extruded into cylinders and spun around the origin

    float outerRadius - The radius of the "circle" from the origin at which the center of the cylinders lie
    float innerRadius - The radius of the cylinders
    int tubeSides - Number of vertices in each circle
    int ringSides - How many cylinder circles to draw around the torus
*/
void genTorus(float outerRadius, float innerRadius, int tubeSides, int ringSides);