#include "meta.h"
#include "objects/folder.h"
#include "objects/joint/jointinstance.h"
#include "objects/joint/rotate.h"
#include "objects/joint/rotatev.h"
#include "objects/joint/weld.h"
#include "objects/service/jointsservice.h"
#include "objects/model.h"
#include "objects/part.h"
#include "objects/joint/snap.h"
#include "objects/script.h"
#include "objects/service/script/scriptcontext.h"
#include "objects/service/script/serverscriptservice.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"

std::map<std::string, const InstanceType*> INSTANCE_MAP = {
    { "Instance", &Instance::TYPE },
    { "DataModel", &DataModel::TYPE },

    { "Part", &Part::TYPE },
    { "Snap", &Snap::TYPE },
    { "Weld", &Weld::TYPE },
    { "Rotate", &Rotate::TYPE },
    { "RotateV", &RotateV::TYPE },
    { "JointInstance", &JointInstance::TYPE },
    { "Script", &Script::TYPE },
    { "Model", &Model::TYPE },
    // { "Folder", &Folder::TYPE },

    // Services

    { "Workspace", &Workspace::TYPE },
    { "JointsService", &JointsService::TYPE },
    { "ScriptContext", &ScriptContext::TYPE },
    { "ServerScriptService", &ServerScriptService::TYPE },
};