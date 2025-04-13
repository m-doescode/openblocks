#include "part.h"
#include "base/instance.h"
#include "common.h"
#include "datatypes/base.h"
#include "datatypes/cframe.h"
#include "datatypes/color3.h"
#include "datatypes/vector.h"
#include "objects/base/member.h"
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
            return *static_cast<Data::CFrame*>(source);
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

Part::Part(PartConstructParams params): Instance(&TYPE), cframe(Data::CFrame(params.position, params.rotation)),
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
            }}
        }
    });
}

Part::~Part() {
    // This relies on physicsCommon still existing. Be very careful.
    if (this->rigidBody && workspace())
        workspace().value()->DestroyRigidBody(rigidBody);
}


void Part::OnAncestryChanged(std::optional<std::shared_ptr<Instance>> child, std::optional<std::shared_ptr<Instance>> newParent) {
    if (this->rigidBody)
        this->rigidBody->setIsActive(workspace().has_value());

    if (workspace())
        workspace().value()->SyncPartPhysics(std::dynamic_pointer_cast<Part>(this->shared_from_this()));

    // TODO: Sleeping bodies that touch this one also need to be updated
}

void Part::onUpdated(std::string property) {
    if (workspace())
        workspace().value()->SyncPartPhysics(std::dynamic_pointer_cast<Part>(this->shared_from_this()));
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