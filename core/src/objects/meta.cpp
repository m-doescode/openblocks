#include "meta.h"

#define DECLTYPE(className) class className { public: const static InstanceType TYPE; };

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
    { "Instance", &Instance::TYPE },
    { "DataModel", &DataModel::TYPE },

    { "BasePart", &BasePart::TYPE },
    { "Part", &Part::TYPE },
    { "WedgePart", &WedgePart::TYPE },
    { "Snap", &Snap::TYPE },
    { "Weld", &Weld::TYPE },
    { "Rotate", &Rotate::TYPE },
    { "RotateV", &RotateV::TYPE },
    { "Motor6D", &Motor6D::TYPE },
    { "JointInstance", &JointInstance::TYPE },
    { "Script", &Script::TYPE },
    { "Model", &Model::TYPE },
    { "Message", &Message::TYPE },
    { "Hint", &Hint::TYPE },
    // { "Folder", &Folder::TYPE },

    // Services

    { "Workspace", &Workspace::TYPE },
    { "JointsService", &JointsService::TYPE },
    { "ScriptContext", &ScriptContext::TYPE },
    { "ServerScriptService", &ServerScriptService::TYPE },
    { "Selection", &Selection::TYPE },
};