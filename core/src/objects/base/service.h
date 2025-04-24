#pragma once

// Services are top-level singletons and belong to a specific DataModel
// They serve one specific task and can be accessed using game:GetService
#include "objects/datamodel.h"
#include <memory>
class Service : public Instance {
protected:
    Service(const InstanceType* type);
    virtual void InitService();
    virtual void OnRun();

    void OnParentUpdated(std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent) override;

    friend class DataModel;
};