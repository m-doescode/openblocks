#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

const int FaceRight = 0;
const int FaceTop = 1;
const int FaceBack = 2;	
const int FaceLeft = 3;	
const int FaceBottom = 4;
const int FaceFront	= 5;
const int FaceNone	= 6;

const int SurfaceSmooth = 0;
const int SurfaceGlue = 1;
const int SurfaceWeld = 2;
const int SurfaceStuds = 3;
const int SurfaceInlets = 4;
const int SurfaceUniversal = 5;

out vec3 vPos;
out vec3 lPos;
out vec3 vNormal;
out vec3 lNormal;
out vec2 vTexCoords;
flat out int vSurfaceZ;

uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform int surfaces[6];
uniform vec3 texScale;

const float faceThreshold = sqrt(2)/2;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vPos = vec3(model * vec4(aPos, 1.0));
    lPos = aPos;
    vNormal = normalMatrix * aNormal;
    lNormal = aNormal;
    int vFace = FaceNone;

    if (dot(vec3(0, 1, 0), aNormal) > faceThreshold)
        vFace = FaceTop;
    else if (dot(vec3(0, -1, 0), aNormal) > faceThreshold)
        vFace = FaceBottom;
    else if (dot(vec3(1, 0, 0), aNormal) > faceThreshold)
        vFace = FaceRight;
    else if (dot(vec3(-1, 0, 0), aNormal) > faceThreshold)
        vFace = FaceLeft;
    else if (dot(vec3(0, 0, -1), aNormal) > faceThreshold)
        vFace = FaceFront;
    else if (dot(vec3(0, 0, 1), aNormal) > faceThreshold)
        vFace = FaceBack;

    vSurfaceZ = surfaces[vFace];
    if (surfaces[vFace] > SurfaceUniversal) vSurfaceZ = 0;
}
