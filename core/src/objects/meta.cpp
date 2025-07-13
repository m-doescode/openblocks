#include "meta.h"
#include "objects/folder.h"
#include "objects/hint.h"
#include "objects/joint/jointinstance.h"
#include "objects/joint/rotate.h"
#include "objects/joint/rotatev.h"
#include "objects/joint/weld.h"
#include "objects/message.h"
#include "objects/service/jointsservice.h"
#include "objects/model.h"
#include "objects/part/part.h"
#include "objects/joint/snap.h"
#include "objects/script.h"
#include "objects/service/script/scriptcontext.h"
#include "objects/service/script/serverscriptservice.h"
#include "objects/service/selection.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"

std::map<std::string, const InstanceType*> INSTANCE_MAP = {
    { "Instance", &Instance::TYPE },
    { "DataModel", &DataModel::TYPE },

    { "BasePart", &BasePart::TYPE },
    { "Part", &Part::TYPE },
    { "Snap", &Snap::TYPE },
    { "Weld", &Weld::TYPE },
    { "Rotate", &Rotate::TYPE },
    { "RotateV", &RotateV::TYPE },
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