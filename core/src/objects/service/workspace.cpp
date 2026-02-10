#include "workspace.h"
#include "datatypes/variant.h"
#include "datatypes/ref.h"
#include "datatypes/vector.h"
#include "logger.h"
#include "objectmodel/property.h"
#include "objects/base/instance.h"
#include "objects/part/part.h"
#include "objects/part/wedgepart.h"
#include "objects/service/jointsservice.h"
#include "objects/joint/jointinstance.h"
#include "objects/datamodel.h"
#include "timeutil.h"
#include <memory>

InstanceType Workspace::__buildType() {
    return make_instance_type<Workspace>("Workspace", INSTANCE_SERVICE | INSTANCE_NOTCREATABLE,
        def_property("fallenPartsDestroyHeight", &Workspace::fallenPartsDestroyHeight)
    );
}

Workspace::Workspace(): physicsWorld(std::make_shared<PhysWorld>()) {
}

Workspace::~Workspace() =  default;

void Workspace::InitService() {
    if (initialized) return;
    initialized = true;

    // Create meshes
    // WedgePart::createWedgeShape(physicsCommon);
}

void Workspace::OnRun() {
    // Make joints
    for (auto it = this->GetDescendantsStart(); it != this->GetDescendantsEnd(); it++) {
        if (!it->IsA<BasePart>()) continue;
        std::shared_ptr<BasePart> part = it->CastTo<BasePart>().expect();
        part->MakeJoints();
    }

    // Activate all joints
    for (auto it = this->GetDescendantsStart(); it != this->GetDescendantsEnd(); it++) {
        std::shared_ptr<Instance> obj = *it;
        if (!obj->IsA<JointInstance>()) continue;
        std::shared_ptr<JointInstance> joint = obj->CastTo<JointInstance>().expect();
        joint->UpdateProperty("Part0");
    }

    for (auto obj : dataModel()->GetService<JointsService>()->GetChildren()) {
        if (!obj->IsA<JointInstance>()) continue;
        std::shared_ptr<JointInstance> joint = obj->CastTo<JointInstance>().expect();
        joint->UpdateProperty("Part0");
    }
}

void Workspace::SyncPartPhysics(std::shared_ptr<BasePart> part) {
    physicsWorld->syncBodyProperties(part);
}

void Workspace::PhysicsStep(float deltaTime) {
    physicsWorld->step(deltaTime);

    for (std::shared_ptr<BasePart> part : physicsWorld->getSimulatedBodies()) {
        // Destroy fallen parts
        if (part->cframe.Position().Y() < this->fallenPartsDestroyHeight) {
            auto parent = part->GetParent();
            part->Destroy();

            // If the parent of the part is a Model, destroy it too
            if (parent != nullptr && parent->IsA("Model"))
                parent->Destroy();
        }
    }
}

std::vector<std::shared_ptr<Instance>> Workspace::CastFrustum(Frustum frustum) {
    std::vector<std::shared_ptr<Instance>> parts;

    for (auto it = GetDescendantsStart(); it != GetDescendantsEnd(); it++) {
        if (!it->IsA<BasePart>()) continue;
        std::shared_ptr<BasePart> part = std::dynamic_pointer_cast<BasePart>(*it);

        if (!part->locked && frustum.checkAABB(part->position(), part->GetAABB())) {
            parts.push_back(part);
        }
    }

    return parts;
}