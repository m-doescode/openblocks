#pragma once

#include "objects/annotation.h"
#include "objects/base/service.h"

class DEF_INST_SERVICE_(hidden) JointsService : public Service {
    AUTOGEN_PREAMBLE
private:
    nullable std::shared_ptr<Workspace> jointWorkspace();
protected:
    void InitService() override;
    bool initialized = false;

public:
    JointsService();
    ~JointsService();

    static inline std::shared_ptr<Instance> Create() { return std::make_shared<JointsService>(); };
};