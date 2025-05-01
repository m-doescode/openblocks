#pragma once

enum NormalId {
    Right = 0,
    Top = 1,
    Back = 2,
    Left = 3,
    Bottom = 4,
    Front = 5
};

enum SurfaceType {
    SurfaceSmooth = 0,
    SurfaceGlue = 1,
    SurfaceWeld = 2,
    SurfaceStuds = 3,
    SurfaceInlets = 4,
    SurfaceUniversal = 5,
    SurfaceHinge = 6,
    SurfaceMotor = 7,
};

namespace Data { class Vector3; } using Data::Vector3;
NormalId faceFromNormal(Vector3);
Vector3 normalFromFace(NormalId);