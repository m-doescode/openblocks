#include "meta.h"
#include "objects/joint/jointinstance.h"
#include "objects/jointsservice.h"
#include "objects/part.h"
#include "objects/joint/snap.h"
#include "objects/workspace.h"

std::map<std::string, const InstanceType*> INSTANCE_MAP = {
    { "Instance", &Instance::TYPE },
    { "Part", &Part::TYPE },
    { "Workspace", &Workspace::TYPE },
    { "DataModel", &DataModel::TYPE },
    { "Snap", &Snap::TYPE },
    { "JointInstance", &JointInstance::TYPE },
    { "JointsService", &JointsService::TYPE },
};