#include "part.h"
#include "base/instance.h"
#include "common.h"
#include "datatypes/base.h"
#include "datatypes/cframe.h"
#include "datatypes/color3.h"
#include "datatypes/vector.h"
#include "objects/base/member.h"
#include "objects/jointsservice.h"
#include "objects/snap.h"
#include "rendering/surface.h"
#include <memory>
#include <optional>

using Data::Vector3;

// template <typename T, typename U>
// constexpr FieldCodec fieldCodecOf() {
//     return FieldCodec {
//         .write = [](Data::Variant source, void* destination) {
//             *(U*)destination = (U)source.get<T>();
//         },
//         .read = [](void* source) -> Data::Variant {
//             return T(*(U*)source);
//         },
//     };
// }

constexpr FieldCodec cframePositionCodec() {
    return FieldCodec {
        .write = [](Data::Variant source, void* destination) {
            Data::CFrame* cframe = static_cast<Data::CFrame*>(destination);
            *cframe = cframe->Rotation() + source.get<Vector3>();
        },
        .read = [](void* source) -> Data::Variant {
            return static_cast<Data::CFrame*>(source)->Position();
        },
    };
}

constexpr FieldCodec cframeRotationCodec() {
    return FieldCodec {
        .write = [](Data::Variant source, void* destination) {
            Data::CFrame* cframe = static_cast<Data::CFrame*>(destination);
            *cframe = Data::CFrame::FromEulerAnglesXYZ(source.get<Vector3>()) + cframe->Position();
        },
        .read = [](void* source) -> Data::Variant {
            return static_cast<Data::CFrame*>(source)->ToEulerAnglesXYZ();
        },
    };
}

const InstanceType Part::TYPE = {
    .super = &Instance::TYPE,
    .className = "Part",
    .constructor = &Part::CreateGeneric,
    .explorerIcon = "part",
};

const InstanceType* Part::GetClass() {
    return &TYPE;
}

Part::Part(): Part(PartConstructParams { .color = Data::Color3(0.639216f, 0.635294f, 0.647059f) }) {
}

Part::Part(PartConstructParams params): Instance(&TYPE), cframe(Data::CFrame::FromEulerAnglesXYZ((Data::Vector3)params.rotation) + params.position),
                                        size(params.size), color(params.color), anchored(params.anchored), locked(params.locked) {                      
    this->memberMap = std::make_unique<MemberMap>(MemberMap {
        .super = std::move(this->memberMap),
        .members = {
            { "Anchored", {
                .backingField = &anchored,
                .type = &Data::Bool::TYPE,
                .codec = fieldCodecOf<Data::Bool, bool>(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this),
                .category = PROP_CATEGORY_BEHAVIOR,
            }}, { "Locked", {
                .backingField = &locked,
                .type = &Data::Bool::TYPE,
                .codec = fieldCodecOf<Data::Bool, bool>(),
                .category = PROP_CATEGORY_BEHAVIOR,
            }}, { "Position", {
                .backingField = &cframe,
                .type = &Vector3::TYPE,
                .codec = cframePositionCodec(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this),
                .flags = PropertyFlags::PROP_NOSAVE
            }}, { "Rotation", {
                .backingField = &cframe,
                .type = &Vector3::TYPE,
                .codec = cframeRotationCodec(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this),
                .flags = PropertyFlags::PROP_NOSAVE
            }}, { "Velocity", {
                .backingField = &velocity,
                .type = &Vector3::TYPE,
                .codec = fieldCodecOf<Data::Vector3>(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this),
            }}, { "CFrame", {
                .backingField = &cframe,
                .type = &Data::CFrame::TYPE,
                .codec = fieldCodecOf<Data::CFrame>(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this),
            }}, { "Size", {
                .backingField = &size,
                .type = &Vector3::TYPE,
                .codec = fieldCodecOf<Vector3, glm::vec3>(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this),
                .category = PROP_CATEGORY_PART,
            }}, { "Color", {
                .backingField = &color,
                .type = &Data::Color3::TYPE,
                .codec = fieldCodecOf<Data::Color3>(),
                .category = PROP_CATEGORY_APPEARENCE,
            }}, { "Transparency", {
                .backingField = &transparency,
                .type = &Data::Float::TYPE,
                .codec = fieldCodecOf<Data::Float, float>(),
                .flags = PROP_UNIT_FLOAT,
                .category = PROP_CATEGORY_APPEARENCE,
            }},

            // Surfaces

            { "TopSurface", {
                .backingField = &topSurface,
                .type = &Data::Int::TYPE, // Replace with enum
                .codec = fieldCodecOf<Data::Int, int>(),
                .flags = PROP_HIDDEN,
                .category = PROP_CATEGORY_SURFACE,
            }}, { "BottomSurface", {
                .backingField = &bottomSurface,
                .type = &Data::Int::TYPE, // Replace with enum
                .codec = fieldCodecOf<Data::Int, int>(),
                .flags = PROP_HIDDEN,
                .category = PROP_CATEGORY_SURFACE,
            }}, { "FrontSurface", {
                .backingField = &frontSurface,
                .type = &Data::Int::TYPE, // Replace with enum
                .codec = fieldCodecOf<Data::Int, int>(),
                .flags = PROP_HIDDEN,
                .category = PROP_CATEGORY_SURFACE,
            }}, { "BackSurface", {
                .backingField = &backSurface,
                .type = &Data::Int::TYPE, // Replace with enum
                .codec = fieldCodecOf<Data::Int, int>(),
                .flags = PROP_HIDDEN,
                .category = PROP_CATEGORY_SURFACE,
            }}, { "RightSurface", {
                .backingField = &rightSurface,
                .type = &Data::Int::TYPE, // Replace with enum
                .codec = fieldCodecOf<Data::Int, int>(),
                .flags = PROP_HIDDEN,
                .category = PROP_CATEGORY_SURFACE,
            }}, { "LeftSurface", {
                .backingField = &leftSurface,
                .type = &Data::Int::TYPE, // Replace with enum
                .codec = fieldCodecOf<Data::Int, int>(),
                .flags = PROP_HIDDEN,
                .category = PROP_CATEGORY_SURFACE,
            }}, 
        }
    });
}

Part::~Part() {
    // This relies on physicsCommon still existing. Be very careful.
    if (this->rigidBody && workspace()) {
        workspace().value()->DestroyRigidBody(rigidBody);
        this->rigidBody = nullptr;
    }
}


void Part::OnAncestryChanged(std::optional<std::shared_ptr<Instance>> child, std::optional<std::shared_ptr<Instance>> newParent) {
    if (this->rigidBody)
        this->rigidBody->setIsActive(workspace().has_value());

    if (workspace())
        workspace().value()->SyncPartPhysics(std::dynamic_pointer_cast<Part>(this->shared_from_this()));

    // Destroy joints
    if (!workspace()) BreakJoints();

    // TODO: Sleeping bodies that touch this one also need to be updated
}

void Part::onUpdated(std::string property) {
    // Reset velocity
    if (property != "Velocity")
        velocity = Data::Vector3::ZERO;

    if (workspace())
        workspace().value()->SyncPartPhysics(std::dynamic_pointer_cast<Part>(this->shared_from_this()));

    // When position/rotation/size is manually edited, break all joints, they don't apply anymore
    BreakJoints();
}

// Expands provided extents to fit point
static void expandMaxExtents(Vector3* min, Vector3* max, Vector3 point) {
    *min = Vector3(glm::min(min->X(), point.X()), glm::min(min->Y(), point.Y()), glm::min(min->Z(), point.Z()));
    *max = Vector3(glm::max(max->X(), point.X()), glm::max(max->Y(), point.Y()), glm::max(max->Z(), point.Z()));
}

static Vector3 verts[8] {
    {-1, -1, -1},
    {-1, -1, 1},
    {-1, 1, -1},
    {-1, 1, 1},
    {1, -1, -1},
    {1, -1, 1},
    {1, 1, -1},
    {1, 1, 1},
};

Vector3 Part::GetAABB() {
    Vector3 min(0, 0, 0);
    Vector3 max(0, 0, 0);
    for (Vector3 vert : verts) {
        Vector3 worldVert = this->cframe.Rotation() * ((Data::Vector3)this->size * vert);
        expandMaxExtents(&min, &max, worldVert);
    }

    return (min - max).Abs() / 2;
}

void Part::BreakJoints() {
    for (std::weak_ptr<Snap> joint : primaryJoints) {
        if (joint.expired()) continue;
        joint.lock()->Destroy();
    }

    for (std::weak_ptr<Snap> joint : secondaryJoints) {
        if (joint.expired()) continue;
        joint.lock()->Destroy();
    }
}

static Data::Vector3 FACES[6] = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
    {-1, 0, 0},
    {0, -1, 0},
    {0, 0, -1},
};

SurfaceType Part::surfaceFromFace(NormalId face) {
    switch (face) {
        case Top: return topSurface;
        case Bottom: return bottomSurface;
        case Right: return rightSurface;
        case Left: return leftSurface;
        case Front: return frontSurface;
        case Back: return backSurface;
    }
    return SurfaceSmooth; // Unreachable
}

void Part::MakeJoints() {
    // Algorithm: Find nearby parts
    // Make sure parts are not dependant on each other (via primary/secondaryJoints)
    // Find matching surfaces (surface normal dot product < -0.999)
    // Get surface cframe of this part
    // Transform surface center of other part to local via surface cframe of this part
    // Make sure z of transformed center is not greater than 0.05

    if (!workspace()) return;

    // TEMPORARY
    // TODO: Use more efficient algorithm to *actually* find nearby parts)
    for (auto it = workspace().value()->GetDescendantsStart(); it != workspace().value()->GetDescendantsEnd(); it++) {
        InstanceRef obj = *it;
        if (obj == shared_from_this()) continue; // Skip ourselves
        if (obj->GetClass()->className != "Part") continue; // TODO: Replace this with a .IsA call instead of comparing the class name directly
        std::shared_ptr<Part> otherPart = obj->CastTo<Part>().expect();

        for (Data::Vector3 myFace : FACES) {
            Data::Vector3 myWorldNormal = cframe.Rotation() * myFace;
            Data::Vector3 validUp = cframe.Rotation() * Data::Vector3(1,1,1).Unit(); // If myFace == (0, 1, 0), then (0, 1, 0) would produce NaN as up, so we fudge the up so that it works
            Data::CFrame surfaceFrame(cframe.Position(), cframe * (myFace * size), validUp);

            for (Data::Vector3 otherFace : FACES) {
                Data::Vector3 otherWorldNormal = otherPart->cframe.Rotation() * otherFace;
                Data::Vector3 otherSurfaceCenter = otherPart->cframe * (otherFace * otherPart->size);
                Data::Vector3 surfacePointLocalToMyFrame = surfaceFrame.Inverse() * otherSurfaceCenter;

                float dot = myWorldNormal.Dot(otherWorldNormal);
                if (dot > -0.99) continue; // Surface is pointing opposite to ours
                if (abs(surfacePointLocalToMyFrame.Z()) > 0.05) continue; // Surfaces are within 0.05 studs of one another

                SurfaceType mySurface = surfaceFromFace(faceFromNormal(myFace));
                SurfaceType otherSurface = surfaceFromFace(faceFromNormal(otherFace));

                if (mySurface == SurfaceSmooth) continue; // We're not responsible for any joints
                else if (mySurface == SurfaceWeld || mySurface == SurfaceGlue
                || (mySurface == SurfaceStuds && (otherSurface == SurfaceInlets || otherSurface == SurfaceUniversal))
                || (mySurface == SurfaceInlets && (otherSurface == SurfaceStuds || otherSurface == SurfaceUniversal))
                || (mySurface == SurfaceUniversal && (otherSurface == SurfaceStuds || otherSurface == SurfaceInlets || otherSurface == SurfaceUniversal))) { // Always make a weld no matter what the other surface is
                    std::shared_ptr<Snap> joint = Snap::New();
                    joint->part0 = shared<Part>();
                    joint->part1 = otherPart->shared<Part>();
                    joint->c1 = cframe;
                    joint->c0 = otherPart->cframe;
                    dataModel().value()->GetService<JointsService>()->AddChild(joint);
                    joint->UpdateProperty("Part0");

                    printf("Made joint between %s and %s!\n", name.c_str(), otherPart->name.c_str());
                }
            }
        }
    }
}

void Part::trackJoint(std::shared_ptr<Snap> joint) {
    if (!joint->part0.expired() && joint->part0.lock() == shared_from_this()) {
        for (auto it = primaryJoints.begin(); it != primaryJoints.end();) {
            // Clean expired refs
            if (it->expired()) {
                primaryJoints.erase(it);
                continue;
            }

            // If the joint is already tracked, skip
            if (it->lock() == joint)
                return;
            it++;
        }

        primaryJoints.push_back(joint);
    } else if (!joint->part1.expired() && joint->part1.lock() == shared_from_this()) {
        for (auto it = secondaryJoints.begin(); it != secondaryJoints.end();) {
            // Clean expired refs
            if (it->expired()) {
                secondaryJoints.erase(it);
                continue;
            }

            // If the joint is already tracked, skip
            if (it->lock() == joint)
                return;
            it++;
        }

        secondaryJoints.push_back(joint);
    }
}

void Part::untrackJoint(std::shared_ptr<Snap> joint) {
    for (auto it = primaryJoints.begin(); it != primaryJoints.end();) {
        // Clean expired refs
        if (it->expired() || it->lock() == joint) {
            primaryJoints.erase(it);
            continue;
        }

        it++;
    }

    for (auto it = secondaryJoints.begin(); it != secondaryJoints.end();) {
        // Clean expired refs
        if (it->expired() || it->lock() == joint) {
            secondaryJoints.erase(it);
            continue;
        }

        it++;
    }
}