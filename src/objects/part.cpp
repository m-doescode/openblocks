#include "part.h"
#include "base/instance.h"
#include "datatypes/base.h"
#include "datatypes/vector.h"
#include "objects/base/member.h"
#include <memory>
#include <optional>
#include "physics/simulation.h"

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

Part::Part(PartConstructParams params): Instance(&TYPE), position(params.position), rotation(params.rotation),
                                        scale(params.scale), material(params.material), anchored(params.anchored) {                      
    this->memberMap = std::make_unique<MemberMap>(MemberMap {
        .super = std::move(this->memberMap),
        .members = {
            { "Anchored", { .backingField = &anchored, .type = &Data::Bool::TYPE, .codec = fieldCodecOf<Data::Bool, bool>(), .updateCallback = memberFunctionOf(&Part::onUpdated, this) } },
            { "Position", { .backingField = &position, .type = &Data::Vector3::TYPE, .codec = fieldCodecOf<Data::Vector3, glm::vec3>(), .updateCallback = memberFunctionOf(&Part::onUpdated, this) } }
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