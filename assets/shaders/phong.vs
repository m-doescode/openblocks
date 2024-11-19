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

const int SurfaceSmooth = 0;
const int SurfaceGlue = 1;
const int SurfaceWeld = 2;
const int SurfaceStuds = 3;
const int SurfaceInlets = 4;
const int SurfaceUniversal = 5;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexCoords;
flat out int vSurfaceZ;

uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform int surfaces[6];
uniform vec3 texScale;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vPos = vec3(model * vec4(aPos, 1.0));
    vNormal = normalMatrix * aNormal;
    int vFace = aNormal == vec3(0,1,0) ? FaceTop :
            aNormal == vec3(0, -1, 0) ? FaceBottom :
            aNormal == vec3(1, 0, 0) ? FaceRight :
            aNormal == vec3(-1, 0, 0) ? FaceLeft :
            aNormal == vec3(0, 0, 1) ? FaceFront :
            aNormal == vec3(0, 0, -1) ? FaceBack : -1;

    vSurfaceZ = surfaces[vFace];
    // if (surfaces[vFace] > SurfaceUniversal) vSurfaceZ = 0;

    switch (vFace) {
    case FaceTop:
    case FaceBottom:
        // vTexCoords = aTexCoords * vec2(texScale.x / 2, fract(surfaceOffset + texScale.z / 12));
        vTexCoords = aTexCoords * vec2(texScale.x, texScale.z) / 2;
        break;
    case FaceLeft:
    case FaceRight:
        vTexCoords = aTexCoords * vec2(texScale.y, texScale.z) / 2;
        break;
    case FaceFront:
    case FaceBack:
        vTexCoords = aTexCoords * vec2(texScale.x, texScale.y) / 2;
        break;
    };
}
