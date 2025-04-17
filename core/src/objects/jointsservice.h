#pragma once

#include "objects/base/service.h"

class JointsService : public Service {
protected:
    void InitService() override;
    bool initialized = false;

public:
    const static InstanceType TYPE;

    JointsService();
    ~JointsService();

    static inline std::shared_ptr<Instance> Create() { return std::make_shared<JointsService>(); };
    virtual const InstanceType* GetClass() override;
};