#include "surface.h"
#include "datatypes/vector.h"

Vector3 FACE_NORMALS[6] = {
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 0, 0, 1 },
    { -1, 0, 0 },
    { 0, -1, 0 },
    { 0, 0, -1 },
};

NormalId faceFromNormal(Vector3 normal) {
    for (int face = 0; face < 6; face++) {
        if (normal.Dot(FACE_NORMALS[face]) > 0.99)
            return (NormalId)face;
    }
    return (NormalId)-1;
}

Vector3 normalFromFace(NormalId face) {
    return FACE_NORMALS[face];
}