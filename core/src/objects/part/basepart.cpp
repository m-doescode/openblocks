#include "basepart.h"
#include "objectmodel/property.h"
#include "objects/base/instance.h"
#include "common.h"
#include "datatypes/base.h"
#include "datatypes/cframe.h"
#include "datatypes/color3.h"
#include "datatypes/vector.h"
#include "objects/base/member.h"
#include "objects/joint/rotate.h"
#include "objects/joint/rotatev.h"
#include "objects/joint/weld.h"
#include "objects/service/jointsservice.h"
#include "objects/joint/jointinstance.h"
#include "objects/joint/snap.h"
#include "rendering/renderer.h"
#include "enum/surface.h"
#include <glm/common.hpp>
#include <memory>

InstanceProperty def_position_property(MemberPropertyListener<BasePart> listener) {
    return {
        "Position", type_meta_of<Vector3>(), 0, "",

        [](std::shared_ptr<Instance> instance) {
            auto obj = std::dynamic_pointer_cast<BasePart>(instance);
            return obj->cframe.Position();
        },
        [](std::shared_ptr<Instance> instance, Variant value) {
            auto obj = std::dynamic_pointer_cast<BasePart>(instance);
            obj->cframe = obj->cframe.Rotation() + value.get<Vector3>();
        },
        [listener](std::shared_ptr<Instance> instance, std::string name, Variant oldValue, Variant newValue) {
            auto obj = std::dynamic_pointer_cast<BasePart>(instance);
            (obj.get()->*listener)(name, oldValue, newValue);
        }
    };
};

InstanceProperty def_rotation_property(MemberPropertyListener<BasePart> listener) {
    return {
        "Rotation", type_meta_of<Vector3>(), 0, "",

        [](std::shared_ptr<Instance> instance) {
            auto obj = std::dynamic_pointer_cast<BasePart>(instance);
            return obj->cframe.Rotation();
        },
        [](std::shared_ptr<Instance> instance, Variant value) {
            auto obj = std::dynamic_pointer_cast<BasePart>(instance);
            obj->cframe = CFrame::FromEulerAnglesXYZ(value.get<Vector3>()) + obj->cframe.Position();
        },
        [listener](std::shared_ptr<Instance> instance, std::string name, Variant oldValue, Variant newValue) {
            auto obj = std::dynamic_pointer_cast<BasePart>(instance);
            (obj.get()->*listener)(name, oldValue, newValue);
        }
    };
};

InstanceType BasePart::__buildType() {
    return make_instance_type<BasePart, PVInstance>("BasePart",
        set_property_category("data"),
        def_property("Velocity", &BasePart::velocity, 0, &BasePart::onUpdated),
        def_property("RotVelocity", &BasePart::rotVelocity, 0, &BasePart::onUpdated),
        def_property("CFrame", &BasePart::cframe, 0, &BasePart::onUpdated),

        def_position_property(&BasePart::onUpdated),
        def_rotation_property(&BasePart::onUpdated),

        set_property_category("part"),
        def_property("Size", &BasePart::size, 0, &BasePart::onUpdated),

        set_property_category("appearance"),
        def_property("Color", &BasePart::color),
        def_property("Transparency", &BasePart::transparency),
        def_property("Reflectance", &BasePart::reflectance),

        set_property_category("behavior"),
        def_property("Anchored", &BasePart::anchored, 0, &BasePart::onUpdated),
        def_property("CanCollide", &BasePart::canCollide, 0, &BasePart::onUpdated),
        def_property("Locked", &BasePart::locked),

        // TODO:
        set_property_category("surface"),
        // def_property("TopSurface", &BasePart::topSurface),
        // def_property("BottomSurface", &BasePart::bottomSurface),
        // def_property("LeftSurface", &BasePart::leftSurface),
        // def_property("RightSurface", &BasePart::rightSurface),
        // def_property("FrontSurface", &BasePart::frontSurface),
        // def_property("BackSurface", &BasePart::backSurface),

        set_property_category("surface input"),
        def_property("TopParamA", &BasePart::topParamA, 0, &BasePart::onParamUpdated),
        def_property("BottomParamA", &BasePart::bottomParamA, 0, &BasePart::onParamUpdated),
        def_property("LeftParamA", &BasePart::leftParamA, 0, &BasePart::onParamUpdated),
        def_property("RightParamA", &BasePart::rightParamA, 0, &BasePart::onParamUpdated),
        def_property("FrontParamA", &BasePart::frontParamA, 0, &BasePart::onParamUpdated),
        def_property("BackParamA", &BasePart::backParamA, 0, &BasePart::onParamUpdated),

        def_property("TopParamB", &BasePart::topParamB, 0, &BasePart::onParamUpdated),
        def_property("BottomParamB", &BasePart::bottomParamB, 0, &BasePart::onParamUpdated),
        def_property("LeftParamB", &BasePart::leftParamB, 0, &BasePart::onParamUpdated),
        def_property("RightParamB", &BasePart::rightParamB, 0, &BasePart::onParamUpdated),
        def_property("FrontParamB", &BasePart::frontParamB, 0, &BasePart::onParamUpdated),
        def_property("BackParamB", &BasePart::backParamB, 0, &BasePart::onParamUpdated)

        // def_signal("Touched", &BasePart::Touched),
        // def_signal("TouchEnded", &BasePart::TouchEnded),
    );
}

BasePart::BasePart() : BasePart(PartConstructParams { .size = glm::vec3(4, 1.2, 2), .color = Color3(0.639216f, 0.635294f, 0.647059f) }) {
}

BasePart::BasePart(PartConstructParams params) : cframe(CFrame::FromEulerAnglesXYZ((Vector3)params.rotation) + params.position),
                                        size(params.size), color(params.color), anchored(params.anchored), locked(params.locked) {
}

BasePart::~BasePart() {
    if (workspace() != nullptr) {
        workspace()->RemoveBody(shared<BasePart>());
    }
}


void BasePart::OnAncestryChanged(nullable std::shared_ptr<Instance> child, nullable std::shared_ptr<Instance> newParent) {
    this->rigidBody.setActive(workspace() != nullptr);

    if (workspace() != nullptr)
        workspace()->SyncPartPhysics(std::dynamic_pointer_cast<BasePart>(this->shared_from_this()));

    // Destroy joints
    if (!workspace()) BreakJoints();

    // TODO: Sleeping bodies that touch this one also need to be updated
}

void BasePart::OnWorkspaceAdded(nullable std::shared_ptr<Workspace> oldWorkspace, std::shared_ptr<Workspace> newWorkspace) {
    newWorkspace->AddBody(shared<BasePart>());
}

void BasePart::OnWorkspaceRemoved(std::shared_ptr<Workspace> oldWorkspace) {
    BreakJoints();
    oldWorkspace->RemoveBody(shared<BasePart>());
}

void BasePart::onUpdated(std::string property, Variant, Variant) {
    bool reset = property == "Position" || property == "Rotation" || property == "CFrame" || property == "Size" || property == "Shape";

    // Sanitize size
    // TODO: Replace this with a validator instead
    if (property == "Size") {
        size = glm::max((glm::vec3)size, glm::vec3(0.1f, 0.1f, 0.1f));
    }
    
    if (workspace() != nullptr)
        workspace()->SyncPartPhysics(std::dynamic_pointer_cast<BasePart>(this->shared_from_this()));

    // When position/rotation/size is manually edited, break all joints, they don't apply anymore
    if (reset)
        BreakJoints();
}

void BasePart::onParamUpdated(std::string property, Variant, Variant) {
    // Send signal to joints to update themselves
    for (std::weak_ptr<JointInstance> joint : primaryJoints) {
        if (joint.expired()) continue;

        joint.lock()->OnPartParamsUpdated();
    }
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

Vector3 BasePart::GetAABB() {
    Vector3 min(0, 0, 0);
    Vector3 max(0, 0, 0);
    for (Vector3 vert : verts) {
        Vector3 worldVert = this->cframe.Rotation() * (this->size * vert);
        expandMaxExtents(&min, &max, worldVert);
    }

    return (min - max).Abs() / 2;
}

void BasePart::BreakJoints() {
    for (std::weak_ptr<JointInstance> joint : primaryJoints) {
        if (joint.expired()) continue;
        joint.lock()->Destroy();
    }

    for (std::weak_ptr<JointInstance> joint : secondaryJoints) {
        if (joint.expired()) continue;
        joint.lock()->Destroy();
    }
}

static Vector3 FACES[6] = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
    {-1, 0, 0},
    {0, -1, 0},
    {0, 0, -1},
};

SurfaceType BasePart::surfaceFromFace(NormalId face) {
    switch (face) {
        case Top: return topSurface;
        case Bottom: return bottomSurface;
        case Right: return rightSurface;
        case Left: return leftSurface;
        case Front: return frontSurface;
        case Back: return backSurface;
    }
    return SurfaceType::Smooth; // Unreachable
}

float BasePart::GetSurfaceParamA(Vector3 face) {
    switch (faceFromNormal(face)) {
        case Top: return topParamA;
        case Bottom: return bottomParamA;
        case Right: return rightParamA;
        case Left: return leftParamA;
        case Front: return frontParamA;
        case Back: return backParamA;
    }
    return 0; // Unreachable
}

float BasePart::GetSurfaceParamB(Vector3 face) {
    switch (faceFromNormal(face)) {
        case Top: return topParamB;
        case Bottom: return bottomParamB;
        case Right: return rightParamB;
        case Left: return leftParamB;
        case Front: return frontParamB;
        case Back: return backParamB;
    }
    return 0; // Unreachable
}

Vector3 BasePart::GetEffectiveSize() {
    return size;
}

bool BasePart::checkJointContinuity(std::shared_ptr<BasePart> otherPart) {
    // Make sure that the two parts don't depend on one another

    return checkJointContinuityUp(otherPart) && checkJointContinuityDown(otherPart);
}

bool BasePart::checkJointContinuityDown(std::shared_ptr<BasePart> otherPart) {
    if (shared<BasePart>() == otherPart) return false;
    for (auto joint : primaryJoints) {
        if (joint.expired() || joint.lock()->part1.expired()) continue;
        if (!joint.lock()->part1.lock()->checkJointContinuityDown(otherPart))
            return false;
    }
    return true;
}

bool BasePart::checkJointContinuityUp(std::shared_ptr<BasePart> otherPart) {
    if (shared<BasePart>() == otherPart) return false;
    for (auto joint : secondaryJoints) {
        if (joint.expired() || joint.lock()->part0.expired()) continue;
        if (!joint.lock()->part0.lock()->checkJointContinuityUp(otherPart))
            return false;
    }
    return true;
}

bool BasePart::checkSurfacesTouching(CFrame surfaceFrame, Vector3 size, Vector3 myFace, Vector3 otherFace, std::shared_ptr<BasePart> otherPart) {
    Vector3 farCorner0 = surfaceFrame.Inverse() * otherPart->cframe * (Vector3::ONE * (otherPart->size / 2.f));
    Vector3 farCorner1 = surfaceFrame.Inverse() * otherPart->cframe * (-Vector3::ONE * (otherPart->size / 2.f));
    
    Vector3 myFarCorner0 = Vector3::ONE * (size / 2.f);
    Vector3 myFarCorner1 = -Vector3::ONE * (size / 2.f);

    // https://stackoverflow.com/a/306332/16255372
    float myTop = glm::max(myFarCorner0.X(), myFarCorner1.X());
    float myBot = glm::min(myFarCorner0.X(), myFarCorner1.X());
    float myRight = glm::max(myFarCorner0.Y(), myFarCorner1.Y());
    float myLeft = glm::min(myFarCorner0.Y(), myFarCorner1.Y());
    float otherTop = glm::max(farCorner0.X(), farCorner1.X());
    float otherBot = glm::min(farCorner0.X(), farCorner1.X());
    float otherRight = glm::max(farCorner0.Y(), farCorner1.Y());
    float otherLeft = glm::min(farCorner0.Y(), farCorner1.Y());

    bool horizOverlap = myLeft < otherRight && myRight > otherLeft;
    bool vertOverlap = myBot < otherTop && myTop > otherBot;

    return horizOverlap && vertOverlap;
}

nullable std::shared_ptr<JointInstance> makeJointFromSurfaces(SurfaceType a, SurfaceType b) {
    if (a == SurfaceType::Weld || b == SurfaceType::Weld || a == SurfaceType::Glue || b == SurfaceType::Glue) return Weld::New();
    if ((a == SurfaceType::Studs && (b == SurfaceType::Inlet || b == SurfaceType::Universal))
    || (a == SurfaceType::Inlet && (b == SurfaceType::Studs || b == SurfaceType::Universal))
    || (a == SurfaceType::Universal && (b == SurfaceType::Studs || b == SurfaceType::Inlet || b == SurfaceType::Universal)))
        return Snap::New();
    if (a == SurfaceType::Hinge)
        return Rotate::New();
    if (a == SurfaceType::Motor)
        return RotateV::New();
    return nullptr;
}

void BasePart::MakeJoints() {
    // Algorithm: Find nearby parts
    // Make sure parts are not dependant on each other (via primary/secondaryJoints)
    // Find matching surfaces (surface normal dot product < -0.999)
    // Get surface cframe of this part
    // Transform surface center of other part to local via surface cframe of this part
    // Make sure z of transformed center is not greater than 0.05

    if (!workspace()) return;

    // TEMPORARY
    // TODO: Use more efficient algorithm to *actually* find nearby parts)
    for (auto it = workspace()->GetDescendantsStart(); it != workspace()->GetDescendantsEnd(); it++) {
        std::shared_ptr<Instance> obj = *it;
        if (obj == shared_from_this()) continue; // Skip ourselves
        if (!obj->IsA<BasePart>()) continue;
        std::shared_ptr<BasePart> otherPart = obj->CastTo<BasePart>().expect();

        for (Vector3 myFace : FACES) {
            Vector3 myWorldNormal = cframe.Rotation() * myFace;
            Vector3 validUp = cframe.Rotation() * Vector3(1,1,1).Unit(); // If myFace == (0, 1, 0), then (0, 1, 0) would produce NaN as up, so we fudge the up so that it works
            CFrame surfaceFrame = CFrame::pointToward(cframe * (myFace * size / 2.f), cframe.Rotation() * myFace);
            Vector3 mySurfaceCenter = cframe * (myFace * size / 2.f);

            for (Vector3 otherFace : FACES) {
                Vector3 otherWorldNormal = otherPart->cframe.Rotation() * otherFace;
                Vector3 otherSurfaceCenter = otherPart->cframe * (otherFace * otherPart->size / 2.f);
                Vector3 surfacePointLocalToMyFrame = surfaceFrame.Inverse() * otherSurfaceCenter;

                float dot = myWorldNormal.Dot(otherWorldNormal);
                if (dot > -0.99) continue; // Surface is pointing opposite to ours

                if (abs(surfacePointLocalToMyFrame.Z()) > 0.05) continue; // Surfaces are within 0.05 studs of one another
                if (!checkSurfacesTouching(surfaceFrame, size, myFace, otherFace, otherPart)) continue; // Surface do not overlap
                if (!checkJointContinuity(otherPart)) continue;

                SurfaceType mySurface = surfaceFromFace(faceFromNormal(myFace));
                SurfaceType otherSurface = surfaceFromFace(faceFromNormal(otherFace));

                // If it is a hinge, only attach if actually touching the "hinge"
                if ((mySurface == SurfaceType::Hinge || mySurface == SurfaceType::Motor) && !checkSurfacesTouching(surfaceFrame, Vector3(0.4, 0.4, 0.4), myFace, otherFace, otherPart)) continue;

                // Create contacts
                // Contact always occurs at the center of Part0's surface (even if that point does not overlap both surfaces)
                // Contact 0 is Part0's contact relative to Part0. It should point *opposite* the direction of its surface normal
                // Contact 1 is Part1's contact relative to Part1. It should point directly toward the direction of its surface normal

                // My additional notes:
                // Contact == Part0.CFrame * C0 == Part1.CFrame * C1
                // C1 == Part1.CFrame:Inverse() * Part0.CFrame * C0
                // Part1.CFrame == Part0.CFrame * C0 * C1:Inverse()
                // C0 == Part0.CFrame:Inverse() * Contact

                CFrame contactPoint = CFrame::pointToward(mySurfaceCenter, -myWorldNormal);
                CFrame contact0 = cframe.Inverse() * contactPoint;
                CFrame contact1 = otherPart->cframe.Inverse() * contactPoint;

                auto joint_ = makeJointFromSurfaces(mySurface, otherSurface);
                if (!joint_) continue;
                std::shared_ptr<JointInstance> joint = joint_;
                joint->part0 = shared<BasePart>();
                joint->part1 = otherPart->shared<BasePart>();
                joint->c0 = contact0;
                joint->c1 = contact1;
                // // If both parts touch directly, this can cause friction in Rotate and RotateV joints, so we leave a little extra space
                // if (joint->IsA("Rotate") || joint->IsA("RotateV"))
                //     joint->c1 = joint->c1 + joint->c1.LookVector() * 0.02f,
                //     joint->c0 = joint->c0 - joint->c0.LookVector() * 0.02f;
                dataModel()->GetService<JointsService>()->AddChild(joint);
                joint->UpdateProperty("Part0");
            }
        }
    }
}

void BasePart::UpdateNoBreakJoints() {    
    if (workspace() != nullptr)
        workspace()->SyncPartPhysics(std::dynamic_pointer_cast<BasePart>(this->shared_from_this()));
}

void BasePart::trackJoint(std::shared_ptr<JointInstance> joint) {
    if (!joint->part0.expired() && joint->part0.lock() == shared_from_this()) {
        for (auto it = primaryJoints.begin(); it != primaryJoints.end();) {
            // Clean expired refs
            if (it->expired()) {
                primaryJoints.erase(it);
                continue;
            }

            // If the joint is already tracked, skip
            if (it->lock() == joint)
                return;
            it++;
        }

        primaryJoints.push_back(joint);
    } else if (!joint->part1.expired() && joint->part1.lock() == shared_from_this()) {
        for (auto it = secondaryJoints.begin(); it != secondaryJoints.end();) {
            // Clean expired refs
            if (it->expired()) {
                secondaryJoints.erase(it);
                continue;
            }

            // If the joint is already tracked, skip
            if (it->lock() == joint)
                return;
            it++;
        }

        secondaryJoints.push_back(joint);
    }
}

void BasePart::untrackJoint(std::shared_ptr<JointInstance> joint) {
    for (auto it = primaryJoints.begin(); it != primaryJoints.end();) {
        // Clean expired refs
        if (it->expired() || it->lock() == joint) {
            primaryJoints.erase(it);
            continue;
        }

        it++;
    }

    for (auto it = secondaryJoints.begin(); it != secondaryJoints.end();) {
        // Clean expired refs
        if (it->expired() || it->lock() == joint) {
            secondaryJoints.erase(it);
            continue;
        }

        it++;
    }
}