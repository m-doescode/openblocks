#pragma once

#include "datatypes/signal.h"
#include "objects/annotation.h"
#include "objects/base/service.h"
#include <memory>
#include <vector>

class DEF_INST_SERVICE_(hidden) Selection : public Service {
    AUTOGEN_PREAMBLE
private:
    std::vector<std::shared_ptr<Instance>> selection;
    protected:
    void InitService() override;
    bool initialized = false;
public:
    Selection();
    ~Selection();
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<Selection>(); };

    std::vector<std::shared_ptr<Instance>> Get();
    void Set(std::vector<std::shared_ptr<Instance>> newSelection);

    void Add(std::vector<std::shared_ptr<Instance>> instances);
    void Remove(std::vector<std::shared_ptr<Instance>> instances);

    SignalSource SelectionChanged;
};