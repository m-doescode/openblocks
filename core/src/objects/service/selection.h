#pragma once

#include "datatypes/signal.h"
#include "objectmodel/macro.h"
#include "objects/base/service.h"
#include <memory>
#include <vector>

class Selection : public Service {
    INSTANCE_HEADER
private:
    std::vector<std::shared_ptr<Instance>> selection;
    protected:
    void InitService() override;
    bool initialized = false;
public:
    ~Selection();
    static inline std::shared_ptr<Instance> Create() { return new_instance<Selection>(); };

    std::vector<std::shared_ptr<Instance>> Get();
    void Set(std::vector<std::shared_ptr<Instance>> newSelection);

    void Add(std::vector<std::shared_ptr<Instance>> instances);
    void Remove(std::vector<std::shared_ptr<Instance>> instances);

    SignalSource SelectionChanged;
};