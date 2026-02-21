#include "serverscriptservice.h"
#include "objectmodel/type.h"
#include "objects/script.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"

INSTANCE_IMPL(ServerScriptService)

InstanceType ServerScriptService::__buildType() {
    return make_instance_type<ServerScriptService>("ServerScriptService", INSTANCE_SERVICE | INSTANCE_NOTCREATABLE,
        set_explorer_icon("server-scripts")
    );
}

ServerScriptService::~ServerScriptService() = default;

void ServerScriptService::InitService() {
    if (initialized) return;
    initialized = true;
}

void ServerScriptService::OnRun() {
    auto workspace = dataModel()->GetService<Workspace>();
    for (auto it = workspace->GetDescendantsStart(); it != workspace->GetDescendantsEnd(); it++) {
        if (!it->IsA<Script>()) continue;
        it->CastTo<Script>().expect()->Run();
    }

    for (auto it = GetDescendantsStart(); it != GetDescendantsEnd(); it++) {
        if (!it->IsA<Script>()) continue;
        it->CastTo<Script>().expect()->Run();
    }
}