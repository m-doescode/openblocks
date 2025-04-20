#pragma once

#include "objects/base/service.h"

class Snap;
class JointsService : public Service {
private:
    std::optional<std::shared_ptr<Workspace>> jointWorkspace();

    friend Snap;
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