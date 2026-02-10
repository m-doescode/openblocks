#pragma once

// Services are top-level singletons and belong to a specific DataModel
// They serve one specific task and can be accessed using game:GetService
#include "objects/base/instance.h"
#include <memory>

class DataModel;

class Service : public Instance {
protected:
    virtual void InitService();
    virtual void OnRun();

    void OnParentUpdated(nullable std::shared_ptr<Instance> oldParent, nullable std::shared_ptr<Instance> newParent) override;

    friend class DataModel;
};