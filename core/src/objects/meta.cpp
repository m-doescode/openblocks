#include "meta.h"

#define DECLTYPE(className) class className { public: static const InstanceType& Type(); };

DECLTYPE(DataModel);
DECLTYPE(BasePart);
DECLTYPE(Part);
DECLTYPE(WedgePart);
DECLTYPE(Snap);
DECLTYPE(Weld);
DECLTYPE(Rotate);
DECLTYPE(RotateV);
DECLTYPE(Motor6D);
DECLTYPE(JointInstance);
DECLTYPE(Script);
DECLTYPE(Model);
DECLTYPE(Message);
DECLTYPE(Hint);
// DECLTYPE(Folder);
DECLTYPE(Workspace);
DECLTYPE(JointsService);
DECLTYPE(ScriptContext);
DECLTYPE(ServerScriptService);
DECLTYPE(Selection);

std::map<std::string, const InstanceType*> INSTANCE_MAP = {
    { "Instance", &Instance::Type() },
    { "DataModel", &DataModel::Type() },

    { "BasePart", &BasePart::Type() },
    { "Part", &Part::Type() },
    { "WedgePart", &WedgePart::Type() },
    { "Snap", &Snap::Type() },
    { "Weld", &Weld::Type() },
    { "Rotate", &Rotate::Type() },
    { "RotateV", &RotateV::Type() },
    { "Motor6D", &Motor6D::Type() },
    { "JointInstance", &JointInstance::Type() },
    { "Script", &Script::Type() },
    { "Model", &Model::Type() },
    { "Message", &Message::Type() },
    { "Hint", &Hint::Type() },
    // { "Folder", &Folder::Type() },

    // Services

    { "Workspace", &Workspace::Type() },
    { "JointsService", &JointsService::Type() },
    { "ScriptContext", &ScriptContext::Type() },
    { "ServerScriptService", &ServerScriptService::Type() },
    { "Selection", &Selection::Type() },
};