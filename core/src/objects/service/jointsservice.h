#pragma once

#include "objectmodel/macro.h"
#include "objects/base/service.h"

class JointsService : public Service {
    INSTANCE_HEADER
private:
    nullable std::shared_ptr<Workspace> jointWorkspace();
protected:
    void InitService() override;
    bool initialized = false;

public:
    ~JointsService();

    static inline std::shared_ptr<Instance> Create() { return new_instance<JointsService>(); };
};