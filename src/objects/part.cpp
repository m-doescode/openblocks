#include "part.h"
#include "base/instance.h"
#include "datatypes/base.h"
#include "objects/base/member.h"
#include <memory>
#include <optional>

static InstanceType TYPE_ {
    .super = Instance::TYPE,
    .className = "Part",
    .constructor = &Part::CreateGeneric,
    .explorerIcon = "part",
};

InstanceType* Part::TYPE = &TYPE_;

InstanceType* Part::GetClass() {
    return &TYPE_;
}

Part::Part(): Part(PartConstructParams {}) {
}

Part::Part(PartConstructParams params): Instance(&TYPE_), position(params.position), rotation(params.rotation),
                                        scale(params.scale), material(params.material), anchored(params.anchored) {
                                            
    this->memberMap = std::make_unique<MemberMap>(MemberMap {
        .super = std::move(this->memberMap),
        .members = {
            { "Anchored", { .backingField = &anchored, .type = &Data::Bool::TYPE, .codec = fieldCodecOf<Data::Bool, bool>() } }
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