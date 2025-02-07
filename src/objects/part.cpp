#include "part.h"
#include "base/instance.h"
#include "datatypes/base.h"
#include "datatypes/cframe.h"
#include "datatypes/vector.h"
#include "objects/base/member.h"
#include <memory>
#include <optional>
#include "physics/simulation.h"

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
            *cframe = cframe->Rotation() + source.get<Data::Vector3>();
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
            *cframe = Data::CFrame::FromEulerAnglesXYZ(source.get<Data::Vector3>()) + cframe->Position();
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

Part::Part(): Part(PartConstructParams {}) {
}

Part::Part(PartConstructParams params): Instance(&TYPE), cframe(Data::CFrame(params.position, params.rotation)),
                                        scale(params.scale), material(params.material), anchored(params.anchored) {                      
    this->memberMap = std::make_unique<MemberMap>(MemberMap {
        .super = std::move(this->memberMap),
        .members = {
            { "Anchored", {
                .backingField = &anchored,
                .type = &Data::Bool::TYPE,
                .codec = fieldCodecOf<Data::Bool, bool>(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this)
            } }, { "Position", {
                .backingField = &cframe,
                .type = &Data::Vector3::TYPE,
                .codec = cframePositionCodec(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this),
                .flags = PropertyFlags::PROP_NOSAVE
            } }, { "Rotation", {
                .backingField = &cframe,
                .type = &Data::Vector3::TYPE,
                .codec = cframeRotationCodec(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this),
                .flags = PropertyFlags::PROP_NOSAVE
            } }, { "CFrame", {
                .backingField = &cframe,
                .type = &Data::CFrame::TYPE,
                .codec = fieldCodecOf<Data::CFrame>(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this),
            } }, { "scale", {
                .backingField = &scale,
                .type = &Data::Vector3::TYPE,
                .codec = fieldCodecOf<Data::Vector3, glm::vec3>(),
                .updateCallback = memberFunctionOf(&Part::onUpdated, this)
            } }
        }
    });
}

// This feels wrong. Get access to PhysicsWorld somehow else? Part will need access to this often though, most likely...
extern rp::PhysicsWorld* world;
Part::~Part() {
    // This relies on physicsCommon still existing. Be very careful.
    if (this->rigidBody)
        world->destroyRigidBody(rigidBody);
}


void Part::OnParentUpdated(std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent) {
    if (this->rigidBody)
        this->rigidBody->setIsActive(newParent.has_value());

    // TODO: Sleeping bodies that touch this one also need to be updated
}

void Part::onUpdated(std::string property) {
    syncPartPhysics(std::dynamic_pointer_cast<Part>(this->shared_from_this()));
}