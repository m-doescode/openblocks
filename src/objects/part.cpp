#include "part.h"
#include "base/instance.h"

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

Part::Part(): Instance(&TYPE_) {
}

Part::Part(PartConstructParams params): Instance(&TYPE_), position(params.position), rotation(params.rotation),
                                        scale(params.scale), material(params.material), anchored(params.anchored) {
}

// This feels wrong. Get access to PhysicsWorld somehow else? Part will need access to this often though, most likely...
extern rp::PhysicsWorld* world;
Part::~Part() {
    world->destroyRigidBody(rigidBody);
    Instance::~Instance();
}


void Part::OnParentUpdated(std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent) {
    if (this->rigidBody)
        this->rigidBody->setIsActive(newParent.has_value());

    // TODO: Sleeping bodies that touch this one also need to be updated
}