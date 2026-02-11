#pragma once

#include "objectmodel/macro.h"
#include "objects/base/service.h"

// Container class for server scripts
// Also handles/manages running server scripts on run
class ServerScriptService : public Service {
    INSTANCE_HEADER
protected:
    bool initialized = false;

public:
    ~ServerScriptService();

    void InitService() override;
    void OnRun() override;
    
    static inline std::shared_ptr<Instance> Create() { return new_instance<ServerScriptService>(); };
};