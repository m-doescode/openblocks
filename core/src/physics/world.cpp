#include "world.h"
#include "datatypes/vector.h"
#include "enum/part.h"
#include "logger.h"
#include "objects/joint/jointinstance.h"
#include "objects/part/basepart.h"
#include "objects/part/part.h"
#include "objects/part/wedgepart.h"
#include "objects/service/workspace.h"
#include "physics/convert.h"
#include "timeutil.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/SubShapeID.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Body/BodyLockInterface.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <algorithm>
#include <cstdio>
#include <memory>

static JPH::TempAllocator* allocator;
static JPH::JobSystem* jobSystem;

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

JPH::Ref<JPH::Shape> wedgeShape;

void physicsInit() {
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    allocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

    // Create special shapes
    JPH::Array<JPH::Vec3> wedgeVerts;
    wedgeVerts.push_back({-1, -1, -1});
    wedgeVerts.push_back({ 1, -1, -1});
    wedgeVerts.push_back({-1, -1,  1});
    wedgeVerts.push_back({ 1, -1,  1});
    wedgeVerts.push_back({ 1,  1,  1});
    wedgeVerts.push_back({-1,  1,  1});
    // // Invisible bevel to avoid phasing
    // wedgeVerts.push_back({1, 1, 0.9});
    // wedgeVerts.push_back({0, 1, 0.9});

    wedgeShape = JPH::ConvexHullShapeSettings(wedgeVerts).Create().Get();	
}

void physicsDeinit() {
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

PhysWorld::PhysWorld() {
    worldImpl.Init(4096, 0, 4096, 4096, broadPhaseLayerInterface, objectBroadPhasefilter, objectLayerPairFilter);
    worldImpl.SetGravity(JPH::Vec3(0, -196, 0));
	JPH::PhysicsSettings settings = worldImpl.GetPhysicsSettings();
	// settings.mPointVelocitySleepThreshold = 0.04f; // Fix parts not sleeping
    // settings.mNumVelocitySteps *= 20;
    // settings.mNumPositionSteps *= 20;
	worldImpl.SetPhysicsSettings(settings);
}

PhysWorld::~PhysWorld() {
}

// void PhysWorld::addBody(std::shared_ptr<BasePart> part) {
//     syncBodyProperties(part);
// }

void PhysWorld::removeBody(std::shared_ptr<BasePart> part) {
    JPH::BodyInterface& interface = worldImpl.GetBodyInterface();

    // https://jrouwe.github.io/JoltPhysics/index.html#sleeping-bodies
    // Wake sleeping bodies in its area before removing it
    Vector3 aabbSize = part->GetAABB();
    interface.ActivateBodiesInAABox(JPH::AABox(convert<JPH::Vec3>(part->position() - aabbSize), convert<JPH::Vec3>(part->position() + aabbSize)), {}, {});

    // interface.RemoveBody(part->rigidBody.bodyImpl->GetID());
    // interface.DestroyBody(part->rigidBody.bodyImpl->GetID());
    // part->rigidBody.bodyImpl = nullptr;
}

JPH::Shape* makeShape(std::shared_ptr<BasePart> basePart) {
    if (std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(basePart)) {
        switch (part->shape) {
        case PartType::Block:
            return new JPH::BoxShape(convert<JPH::Vec3>(part->size / 2.f), JPH::cDefaultConvexRadius);
        case PartType::Ball:
            return new JPH::SphereShape(glm::min(part->size.X(), part->size.Y(), part->size.Z()) / 2.f);
        case PartType::Cylinder:
            return new JPH::RotatedTranslatedShape(JPH::Vec3(), JPH::Quat::sEulerAngles(JPH::Vec3(0, 0, JPH::JPH_PI * 0.5)), new JPH::CylinderShape(part->size.X() / 2.f, glm::min(part->size.Z(), part->size.Y()) / 2.f));
        }
    } else if (std::shared_ptr<WedgePart> part = std::dynamic_pointer_cast<WedgePart>(basePart)) {
        return new JPH::ScaledShape(wedgeShape, convert<JPH::Vec3>(part->size / 2.f));
    }
    return nullptr;
}

// void PhysWorld::syncBodyProperties(std::shared_ptr<BasePart> part) {
//     JPH::BodyInterface& interface = worldImpl.GetBodyInterface();

//     JPH::EMotionType motionType = part->anchored ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
//     JPH::EActivation activationMode = part->anchored ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;
//     JPH::ObjectLayer objectLayer = !part->canCollide ? Layers::NOCOLLIDE : (part->anchored ? Layers::ANCHORED : Layers::DYNAMIC);

//     JPH::Body* body = part->rigidBody.bodyImpl;

//     // Generate a new rigidBody
//     if (body == nullptr) {
//         JPH::Shape* shape = makeShape(part);
//         JPH::BodyCreationSettings settings(shape, convert<JPH::Vec3>(part->position()), convert<JPH::Quat>((glm::quat)part->cframe.RotMatrix()), motionType, objectLayer);
//         settings.mAllowDynamicOrKinematic = true;
//         settings.mRestitution = 0.5;

//         body = interface.CreateBody(settings);
//         body->SetUserData((JPH::uint64)part.get());
//         part->rigidBody.bodyImpl = body;

//         interface.AddBody(body->GetID(), activationMode);
//         interface.SetLinearVelocity(body->GetID(), convert<JPH::Vec3>(part->velocity));
//     } else {
//         std::shared_ptr<Part> part2 = std::dynamic_pointer_cast<Part>(part);
//         bool shouldUpdateShape = (part2 != nullptr && part->rigidBody._lastShape != part2->shape) || part->rigidBody._lastSize == part->size;

//         if (shouldUpdateShape) {
//             // const JPH::Shape* oldShape = body->GetShape();
//             JPH::Shape* newShape = makeShape(part);

//             interface.SetShape(body->GetID(), newShape, true, activationMode);
//             // Seems like Jolt manages its memory for us, so we don't need the below
//             // delete oldShape;
//         }

//         interface.SetObjectLayer(body->GetID(), objectLayer);        
//         interface.SetMotionType(body->GetID(), motionType, activationMode);
//         interface.SetPositionRotationAndVelocity(body->GetID(), convert<JPH::Vec3>(part->position()), convert<JPH::Quat>((glm::quat)part->cframe.RotMatrix()), convert<JPH::Vec3>(part->velocity), /* Angular velocity is NYI: */ body->GetAngularVelocity());
//     }

//     part->rigidBody._lastSize = part->size;
//     if (std::shared_ptr<Part> part2 = std::dynamic_pointer_cast<Part>(part)) part->rigidBody._lastShape = part2->shape;
// }

tu_time_t physTime;
void PhysWorld::step(float deltaTime) {
    tu_time_t startTime = tu_clock_micros();
    // Depending on the load, it may be necessary to call this with a differing collision step count
    // 5 seems to be a good number supporting the high gravity
    worldImpl.Update(deltaTime, 5, allocator, jobSystem);

    JPH::BodyInterface& interface = worldImpl.GetBodyInterface();
    JPH::BodyIDVector bodyIDs;
    worldImpl.GetBodies(bodyIDs);
    for (JPH::BodyID bodyID : bodyIDs) {
        PhysBody* bodyObj = (PhysBody*)interface.GetUserData(bodyID);
        bodyObj->updatePartFrames();
    }

    // Update joints
    for (std::shared_ptr<JointInstance> joint : drivenJoints) {
        joint->OnPhysicsStep(deltaTime);
    }

    physTime = tu_clock_micros() - startTime;
}

PhysJoint PhysWorld::createJoint(PhysJointInfo& type, std::shared_ptr<BasePart> part0, std::shared_ptr<BasePart> part1) {
    // if (part0->rigidBody.bodyImpl == nullptr
    //     || part1->rigidBody.bodyImpl == nullptr
    //     || !part0->workspace()
    //     || !part1->workspace()
    //     || part0->workspace()->physicsWorld != shared_from_this()
    //     || part1->workspace()->physicsWorld != shared_from_this()
    // ) { Logger::fatalError("Failed to create joint between two parts due to the call being invalid"); panic(); };

    // JPH::TwoBodyConstraint* constraint;
    // if (PhysFixedJointInfo* info = dynamic_cast<PhysFixedJointInfo*>(&type)) {
    //     JPH::FixedConstraintSettings settings;
    //     settings.mSpace = JPH::EConstraintSpace::LocalToBodyCOM;
    //     settings.mPoint1 = convert<JPH::Vec3>(info->c0.Position());
    //     settings.mAxisX1 = convert<JPH::Vec3>(info->c0.RightVector());
    //     settings.mAxisY1 = convert<JPH::Vec3>(info->c0.UpVector());
    //     settings.mPoint2 = convert<JPH::Vec3>(info->c1.Position());
    //     settings.mAxisX2 = convert<JPH::Vec3>(info->c1.RightVector());
    //     settings.mAxisY2 = convert<JPH::Vec3>(info->c1.UpVector());
    //     constraint = settings.Create(*part0->rigidBody.bodyImpl, *part1->rigidBody.bodyImpl);
    // } else if (PhysRotatingJointInfo* info = dynamic_cast<PhysRotatingJointInfo*>(&type)) {
    //     JPH::HingeConstraintSettings settings;
    //     settings.mSpace = JPH::EConstraintSpace::LocalToBodyCOM;
    //     settings.mPoint1 = convert<JPH::Vec3>(info->c0.Position());
    //     settings.mNormalAxis1 = convert<JPH::Vec3>(info->c0.RightVector());
    //     settings.mHingeAxis1 = convert<JPH::Vec3>(info->c0.LookVector());
    //     settings.mPoint2 = convert<JPH::Vec3>(info->c1.Position());
    //     settings.mNormalAxis2 = convert<JPH::Vec3>(info->c1.RightVector());
    //     settings.mHingeAxis2 = convert<JPH::Vec3>(info->c1.LookVector());
        
    //     // settings for Motor6D
    //     settings.mMotorSettings.mSpringSettings.mFrequency = 20;
    //     settings.mMotorSettings.mSpringSettings.mDamping = 1;
    //     constraint = settings.Create(*part0->rigidBody.bodyImpl, *part1->rigidBody.bodyImpl);
        
    //     if (PhysMotorizedJointInfo* info = dynamic_cast<PhysMotorizedJointInfo*>(&type)) {
    //         static_cast<JPH::HingeConstraint*>(constraint)->SetMotorState(JPH::EMotorState::Velocity);
    //         static_cast<JPH::HingeConstraint*>(constraint)->SetTargetAngularVelocity(-info->initialVelocity);
    //     } else if (PhysStepperJointInfo* _ = dynamic_cast<PhysStepperJointInfo*>(&type)) {
    //         static_cast<JPH::HingeConstraint*>(constraint)->SetMotorState(JPH::EMotorState::Position);
    //     }
    // } else {
    //     panic();
    // }

    // worldImpl.AddConstraint(constraint);
    // return { constraint, this };
}

void PhysWorld::trackDrivenJoint(std::shared_ptr<JointInstance> motor) {
    drivenJoints.push_back(motor);
}

void PhysWorld::untrackDrivenJoint(std::shared_ptr<JointInstance> motor) {
    for (auto it = drivenJoints.begin(); it != drivenJoints.end();) {
        if (*it == motor)
            it = drivenJoints.erase(it);
        else
            it++;
    }
}

// WATCH OUT! This should only be called for HingeConstraints.
// Can't use dynamic_cast because TwoBodyConstraint is not virtual
void PhysJoint::setAngularVelocity(float velocity) {
    JPH::HingeConstraint* constraint = static_cast<JPH::HingeConstraint*>(jointImpl);
    constraint->SetTargetAngularVelocity(-velocity);
}

void PhysJoint::setTargetAngle(float angle) {
    JPH::HingeConstraint* constraint = static_cast<JPH::HingeConstraint*>(jointImpl);
    constraint->SetTargetAngle(angle);

    // Wake up the part as it could be sleeping
    JPH::BodyInterface& interface = parentWorld->worldImpl.GetBodyInterface();
    JPH::BodyID bodies[] = {constraint->GetBody1()->GetID(), constraint->GetBody2()->GetID()};
    interface.ActivateBodies(bodies, 2);
}

void PhysWorld::destroyJoint(PhysJoint joint) {
    worldImpl.RemoveConstraint(joint.jointImpl);
}

class PhysRayCastBodyFilter : public JPH::BodyFilter {
    bool ShouldCollideLocked(const JPH::Body &inBody) const override {
        // std::shared_ptr<BasePart> part = ((Instance*)inBody.GetUserData())->shared<BasePart>();

        // // Ignore specifically "hidden" parts from raycast
        // // TODO: Replace this with a better system... Please.
        // if (!part->rigidBody.isCollisionsEnabled()) return false;
        // panic(); // TODO:

        return true;
    }
};

std::optional<const RaycastResult> PhysWorld::castRay(Vector3 point, Vector3 rotation, float maxLength, std::optional<RaycastFilter> filter, unsigned short categoryMaskBits) {
    if (filter != std::nullopt) { Logger::fatalError("The filter property of PhysWorld::castRay is not yet implemented"); panic(); };

    const JPH::BodyLockInterface& lockInterface = worldImpl.GetBodyLockInterfaceNoLock();
    const JPH::BodyInterface& interface = worldImpl.GetBodyInterface();
    const JPH::NarrowPhaseQuery& query = worldImpl.GetNarrowPhaseQuery();

    // First we cast a ray to find a matching part
    Vector3 end = point + rotation.Unit() * maxLength;
    JPH::RRayCast ray { convert<JPH::Vec3>(point), convert<JPH::Vec3>(end) };
    JPH::RayCastResult result;
    PhysRayCastBodyFilter bodyFilter;
    bool hitFound = query.CastRay(ray, result, {}, {}, bodyFilter);
    
    // No matches found, return empty
    if (!hitFound) return std::nullopt;

    // Next we cast a ray to find the hit surface and its world position and normal
    JPH::BodyID hitBodyId = result.mBodyID;
    PhysBody* bodyObj = (PhysBody*)interface.GetUserData(hitBodyId);
    std::shared_ptr<BasePart> part = bodyObj->getPartFromShapeId(result.mSubShapeID2);
    const JPH::Shape* shape = interface.GetShape(hitBodyId);

    // Find the hit position and hence the surface normal of the shape at that specific point
    Vector3 hitPosition = point + rotation.Unit() * (maxLength * result.mFraction);
    JPH::Vec3 surfaceNormal = shape->GetSurfaceNormal(result.mSubShapeID2, convert<JPH::Vec3>(part->cframe.Inverse() * hitPosition));
    Vector3 worldNormal = part->cframe.Rotation() * convert<Vector3>(surfaceNormal);

    return RaycastResult {
        .worldPoint = hitPosition,
        .worldNormal = worldNormal,
        .body = lockInterface.TryGetBody(hitBodyId),
        .hitPart = part,
    };
}

uint BroadPhaseLayerInterface::GetNumBroadPhaseLayers() const {
    return BPLayers::NUM_LAYERS;
}

JPH::BroadPhaseLayer BroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const {
    switch (inLayer) {
        case Layers::DYNAMIC: return BPLayers::DYNAMIC;
        case Layers::ANCHORED: return BPLayers::ANCHORED;
        case Layers::NOCOLLIDE: return BPLayers::NOCOLLIDE;
        default: panic();
    }
}

const char * BroadPhaseLayerInterface::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const {
    using T = JPH::BroadPhaseLayer::Type;
    switch ((T)inLayer) {
        case (T)BPLayers::DYNAMIC: return "DYNAMIC";
        case (T)BPLayers::ANCHORED: return "ANCHORED";
        case (T)BPLayers::NOCOLLIDE: return "NOCOLLIDE";
        default: panic();
    }
}

bool ObjectBroadPhaseFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const {
    using T = JPH::BroadPhaseLayer::Type;
    switch ((T)inLayer2) {
    case (T)BPLayers::DYNAMIC: return true;
    case (T)BPLayers::ANCHORED: return true;
    case (T)BPLayers::NOCOLLIDE: return false;
    default: panic();
    }
}

bool ObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const {
    switch (inLayer1) {
    case Layers::DYNAMIC:
        return true;
    case Layers::ANCHORED:
        return inLayer2 == Layers::DYNAMIC;
    case Layers::NOCOLLIDE:
        return false;
    default:
        panic();
    }
}