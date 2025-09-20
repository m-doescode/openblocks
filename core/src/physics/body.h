#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <vector>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "Jolt/Physics/Collision/Shape/MutableCompoundShape.h"
#include "datatypes/cframe.h"
#include "utils.h"

class Workspace;
class BasePart;
class JointInstance;
class PhysBody;

struct PhysBodyHandle {
    std::shared_ptr<PhysBody> body;
    int subShapeId = -1; // Id of the collision shape that represents this BasePart
};

class PhysBody : public std::enable_shared_from_this<PhysBody> {
    std::weak_ptr<Workspace> workspaceRef; // Reference to the workspace of said world, to check if world is null or not
    JPH::Body* body; // Constructed body
    JPH::MutableCompoundShape* rootShape;
    
    int anchoredParts; // A "ref-counting" way of checking if the body should be static or not
    std::vector<std::weak_ptr<BasePart>> constituentParts; // Parts that make up this physics body
    std::map<uint32_t, std::weak_ptr<BasePart>> shapeMap; // Maps shape ids to parts
    std::vector<std::weak_ptr<JointInstance>> internalJoints; // Any/all joints that join parts into this body
    std::vector<std::weak_ptr<JointInstance>> externalJoints; // Joints from this body to other bodies

    JPH::PhysicsSystem* world(); // Pointer to the world that body belongs to, if non-null
public:
    // Destroy the body
    ~PhysBody();

    // Creates a new body containing just the specified part
    static void createBodyFor(std::shared_ptr<BasePart> firstPart);

    // Tells the body to rebuild all shapes from scratch. This is usually only necessary when first creating the body
    void rebuildBody();

    // Tells the body to create or rebuild the shape for a specific part.
    void rebuildShape(std::shared_ptr<BasePart>);

    // Triggers update to aspects of the whole body (usually, whether it is anchored or not)
    void updateBody();

    void addPart(std::shared_ptr<BasePart>, std::shared_ptr<JointInstance> joint);
    void removePart(std::shared_ptr<BasePart>, std::shared_ptr<JointInstance> brokenJoint);

    // Tells the body to dissolve itself and move all of its constituent parts into the specified new body
    void dissolveInto(std::shared_ptr<PhysBody>);

    // Informs the body that a joint connecting two parts has been broken, and lets it decide whether to break off into two separate bodies
    void breakAround(std::shared_ptr<BasePart>, std::shared_ptr<BasePart>, std::shared_ptr<JointInstance> brokenJoint);

    // Checks whether a part is a member of this body
    bool isWeldedTo(std::shared_ptr<BasePart>);

    // If the part is no longer a member of the same workspace, then remove it
    void updatePartMembership(std::shared_ptr<BasePart>);
    
    // Updates the CFrames of member parts to reflect their positions within the rigid body
    void updatePartFrames();

    std::shared_ptr<BasePart> getPartFromShapeId(JPH::SubShapeID subShapeId);
};