#pragma once

#include "objects/annotation.h"
#include "objects/base/service.h"

class INSTANCE_SERVICE() JointsService : public Service {
    AUTOGEN_PREAMBLE
private:
    std::optional<std::shared_ptr<Workspace>> jointWorkspace();
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