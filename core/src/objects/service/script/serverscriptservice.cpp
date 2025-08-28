#include "serverscriptservice.h"
#include "objects/script.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"

ServerScriptService::ServerScriptService(): Service(&TYPE) {
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