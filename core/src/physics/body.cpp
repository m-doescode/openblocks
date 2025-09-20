#include "body.h"

#include <algorithm>
#include <memory>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>

#include "Jolt/Physics/Collision/Shape/SubShapeID.h"
#include "Jolt/Physics/EActivation.h"
#include "datatypes/vector.h"
#include "objects/part/basepart.h"
#include "objects/part/part.h"
#include "objects/part/wedgepart.h"
#include "objects/service/workspace.h"
#include "physics/convert.h"

namespace Layers
{
	static constexpr JPH::ObjectLayer DYNAMIC = 0;
	static constexpr JPH::ObjectLayer ANCHORED = 1;
	static constexpr JPH::ObjectLayer NOCOLLIDE = 2;
	// static constexpr JPH::ObjectLayer NUM_LAYERS = 3;
};

namespace BPLayers
{
	static constexpr JPH::BroadPhaseLayer ANCHORED(0);
	static constexpr JPH::BroadPhaseLayer DYNAMIC(1);
	static constexpr JPH::BroadPhaseLayer NOCOLLIDE(2);
	static constexpr uint NUM_LAYERS(3);
};

extern JPH::Ref<JPH::Shape> wedgeShape;

// Destroy the body
PhysBody::~PhysBody() {
    if (!workspaceRef.expired())
        world()->GetBodyInterface().RemoveBody(body->GetID());
}

// Creates a new body containing just the specified part
void PhysBody::createBodyFor(std::shared_ptr<BasePart> firstPart) {
    std::shared_ptr<PhysBody> body = std::make_shared<PhysBody>();
    body->constituentParts.push_back(firstPart);
    body->rebuildBody();
    firstPart->bodyHandle.body = body;
}

JPH::PhysicsSystem* PhysBody::world() {
    return &workspaceRef.lock()->physicsWorld->worldImpl;
}


// Tells the body to rebuild all shapes from scratch. This is usually only necessary when first creating the body
void PhysBody::rebuildBody() {
    if (constituentParts.size() == 0) return;

    std::shared_ptr<BasePart> rootPart = constituentParts[0].lock();

    // Use the first part's position as the "origin"
    CFrame origin = rootPart->cframe;

    workspaceRef = rootPart->workspace();

    // Calculate number of anchored parts
    anchoredParts = 0;
    for (auto part : constituentParts) {
        if (part.lock()->anchored)
            anchoredParts++;
    }

    // Determine which layer to use based on whether the part is anchored
    JPH::EMotionType motionType = anchoredParts > 0 ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
    JPH::ObjectLayer objectLayer = anchoredParts > 0 ? Layers::ANCHORED : Layers::DYNAMIC;
    
    // Create the body located at the origin
    JPH::BodyInterface& bodyInterface = world()->GetBodyInterface();
    rootShape = new JPH::MutableCompoundShape();
    JPH::BodyCreationSettings settings(
        rootShape,
        convert<JPH::Vec3>(origin.Position()),
        convert<JPH::Quat>((glm::quat)origin.RotMatrix()),
        motionType, objectLayer);
    settings.mAllowDynamicOrKinematic = true;

    // Build shapes for all parts
    for (auto part : constituentParts) {
        rebuildShape(part.lock());
    }

    body = bodyInterface.CreateBody(settings);
    body->SetUserData((uint64_t)this);
    bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);
}

// Tells the body to create or rebuild the shape for a specific part.
void PhysBody::rebuildShape(std::shared_ptr<BasePart> basePart) {
    JPH::Shape* newShape;

    // CFrame bodyFrame = CFrame(convert<Vector3>(body->GetPosition()), convert<glm::quat>(body->GetRotation()));
    CFrame bodyFrame = body == nullptr ? constituentParts[0].lock()->cframe : CFrame(convert<Vector3>(body->GetPosition()), convert<glm::quat>(body->GetRotation()));

    // The frame of this part relative to the origin of the body
    CFrame relativeFrame = bodyFrame.Inverse() * basePart->cframe;

    if (std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(basePart)) {
        switch (part->shape) {
        case PartType::Block:
            newShape = new JPH::BoxShape(convert<JPH::Vec3>(part->size / 2.f), JPH::cDefaultConvexRadius);
        case PartType::Ball:
            newShape = new JPH::SphereShape(glm::min(part->size.X(), part->size.Y(), part->size.Z()) / 2.f);
        case PartType::Cylinder:
            newShape = new JPH::RotatedTranslatedShape(JPH::Vec3(), JPH::Quat::sEulerAngles(JPH::Vec3(0, 0, JPH::JPH_PI * 0.5)), new JPH::CylinderShape(part->size.X() / 2.f, glm::min(part->size.Z(), part->size.Y()) / 2.f));
        }
    } else if (std::shared_ptr<WedgePart> part = std::dynamic_pointer_cast<WedgePart>(basePart)) {
        newShape = new JPH::ScaledShape(wedgeShape, convert<JPH::Vec3>(part->size / 2.f));
    }

    // TODO: May cause race condition. Investigate
    // Add or modify shape
    if (basePart->bodyHandle.subShapeId == -1) {
        basePart->bodyHandle.subShapeId = rootShape->AddShape(
            convert<JPH::Vec3>(relativeFrame.Position()),
            convert<JPH::Quat>((glm::quat)relativeFrame.RotMatrix()),
            newShape
        );
        shapeMap[basePart->bodyHandle.subShapeId] = basePart;
    } else {
        rootShape->ModifyShape(
            basePart->bodyHandle.subShapeId,
            convert<JPH::Vec3>(relativeFrame.Position()),
            convert<JPH::Quat>((glm::quat)relativeFrame.RotMatrix()),
            newShape
        );
    }
}

// Triggers update to aspects of the whole body (usually, whether it is anchored or not)
void PhysBody::updateBody() {
    JPH::BodyInterface& bodyInterface = world()->GetBodyInterface();

    // Determine which layer to use based on whether the part is anchored
    JPH::EMotionType motionType = anchoredParts > 0 ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
    JPH::ObjectLayer objectLayer = anchoredParts > 0 ? Layers::ANCHORED : Layers::DYNAMIC;
    
    bodyInterface.SetMotionType(body->GetID(), motionType, JPH::EActivation::Activate);
    bodyInterface.SetObjectLayer(body->GetID(), objectLayer);
}

void PhysBody::addPart(std::shared_ptr<BasePart> part, std::shared_ptr<JointInstance> joint) {
    internalJoints.push_back(joint);
    constituentParts.push_back(part);
    rebuildShape(part);

    part->bodyHandle.body = shared_from_this();
}

void PhysBody::removePart(std::shared_ptr<BasePart> part, std::shared_ptr<JointInstance> brokenJoint) {
    for (auto it = internalJoints.begin(); it != internalJoints.end();) {
        if ((*it).expired() || it->lock() == brokenJoint)
            internalJoints.erase(it);
        else
            it++;
    }

    for (auto it = constituentParts.begin(); it != constituentParts.end();) {
        if ((*it).expired() || it->lock() == part)
            constituentParts.erase(it);
        else
            it++;
    }

    // TODO: May cause race condition. Investigate
    rootShape->RemoveShape(part->bodyHandle.subShapeId);
    part->bodyHandle = { nullptr, -1 };
}

// Tells the body to dissolve itself and move all of its constituent parts into the specified new body
void dissolveInto(std::shared_ptr<PhysBody>);

// Informs the body that a joint connecting two parts has been broken, and lets it decide whether to break off into two separate bodies
void breakAround(std::shared_ptr<BasePart>, std::shared_ptr<BasePart>, std::shared_ptr<JointInstance> brokenJoint);

// Checks whether a part is a member of this body
bool isWeldedTo(std::shared_ptr<BasePart>);

// If the part is no longer a member of the same workspace, then remove it
void PhysBody::updatePartMembership(std::shared_ptr<BasePart> part) {
    // Nothing to do
    if (part->workspace() == workspaceRef.lock())
        return;

    removePart(part, {});
    if (part->workspace() != nullptr)
        PhysBody::createBodyFor(part);
}

void PhysBody::updatePartFrames() {
    CFrame bodyFrame = CFrame(convert<Vector3>(body->GetPosition()), convert<glm::quat>(body->GetRotation()));

    for (auto entry : shapeMap) {
        uint32_t shapeId = entry.first;
        std::shared_ptr<BasePart> part = entry.second.lock();

        const JPH::CompoundShape::SubShape subShape = rootShape->GetSubShape(shapeId);
        part->cframe = bodyFrame * CFrame(convert<Vector3>(subShape.GetPositionCOM()), convert<glm::quat>(subShape.GetRotation()));
    }
}

std::shared_ptr<BasePart> PhysBody::getPartFromShapeId(JPH::SubShapeID subShapeId) {
    JPH::SubShapeID remainder;
    uint32_t index = rootShape->GetSubShapeIndexFromID(subShapeId, remainder);
    return shapeMap[index].lock();
}
