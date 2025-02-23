#pragma once

#include "base.h"
#include "datatypes/cframe.h"
#include "objects/base/service.h"
#include "objects/part.h"
#include <array>
#include <memory>
#include <reactphysics3d/body/RigidBody.h>

class HandleFace {
    HandleFace(int index, glm::vec3 normal) : index(index), normal(normal){}

    public:
    int index;
    glm::vec3 normal;

    static HandleFace XPos;
    static HandleFace XNeg;
    static HandleFace YPos;
    static HandleFace YNeg;
    static HandleFace ZPos;
    static HandleFace ZNeg;
    static std::array<HandleFace, 6> Faces;
};

class Handles : public Instance {
public:
    const static InstanceType TYPE;

    bool active;
    std::optional<std::weak_ptr<Part>> adornee;
    // inline std::optional<std::weak_ptr<Part>> GetAdornee() { return adornee; }
    // inline void SetAdornee(std::optional<std::weak_ptr<Part>> newAdornee) { this->adornee = newAdornee; updateAdornee(); };

    Handles();

    // World-space handles vs local-space handles
    bool worldMode = false;
    Data::CFrame GetCFrameOfHandle(HandleFace face);
    Data::CFrame PartCFrameFromHandlePos(HandleFace face, Data::Vector3 newPos);
    std::optional<HandleFace> RaycastHandle(rp3d::Ray ray);

    static inline std::shared_ptr<Handles> New() { return std::make_shared<Handles>(); };
    virtual const InstanceType* GetClass() override;
};