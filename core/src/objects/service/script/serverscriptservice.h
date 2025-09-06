#pragma once

#include "objects/annotation.h"
#include "objects/base/service.h"

// Container class for server scripts
// Also handles/manages running server scripts on run
class DEF_INST_SERVICE_(explorer_icon="server-scripts", hidden) ServerScriptService : public Service {
    AUTOGEN_PREAMBLE
protected:
    bool initialized = false;

public:
    ServerScriptService();
    ~ServerScriptService();

    void InitService() override;
    void OnRun() override;
    
    static inline std::shared_ptr<Instance> Create() { return std::make_shared<ServerScriptService>(); };
};