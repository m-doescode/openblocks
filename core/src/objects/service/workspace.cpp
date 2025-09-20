#include "workspace.h"
#include "datatypes/variant.h"
#include "datatypes/ref.h"
#include "datatypes/vector.h"
#include "logger.h"
#include "objects/base/instance.h"
#include "objects/part/part.h"
#include "objects/part/wedgepart.h"
#include "objects/service/jointsservice.h"
#include "objects/joint/jointinstance.h"
#include "objects/datamodel.h"
#include "timeutil.h"
#include <memory>

Workspace::Workspace(): Service(&TYPE), physicsWorld(std::make_shared<PhysWorld>()) {
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

// void Workspace::SyncPartPhysics(std::shared_ptr<BasePart> part) {
//     physicsWorld->syncBodyProperties(part);
// }

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