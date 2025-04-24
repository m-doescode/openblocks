#pragma once

#include "objects/base/service.h"

// Container class for server scripts
// Also handles/manages running server scripts on run
class ServerScriptService : public Service {
private:
protected:
    void InitService() override;
    void OnRun() override;
    bool initialized = false;

public:
    const static InstanceType TYPE;

    ServerScriptService();
    ~ServerScriptService();

    static inline std::shared_ptr<Instance> Create() { return std::make_shared<ServerScriptService>(); };
    virtual const InstanceType* GetClass() override;
};