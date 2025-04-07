#include "surface.h"
#include "datatypes/vector.h"

static std::array<Data::Vector3, 6> FACE_NORMALS = {{
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 0, 0, 1 },
    { -1, 0, 0 },
    { 0, -1, 0 },
    { 0, 0, -1 },
}};

NormalId faceFromNormal(Data::Vector3 normal) {
    for (int face = 0; face < 6; face++) {
        if (normal.Dot(FACE_NORMALS[face]) > 0.99)
            return (NormalId)face;
    }
    return (NormalId)-1;
}

Data::Vector3 normalFromFace(NormalId face) {
    return FACE_NORMALS[face];
}