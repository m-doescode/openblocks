#include "meta.h"
#include "objects/part.h"
#include "objects/snap.h"
#include "objects/workspace.h"

std::map<std::string, const InstanceType*> INSTANCE_MAP = {
    { "Instance", &Instance::TYPE },
    { "Part", &Part::TYPE },
    { "Workspace", &Workspace::TYPE },
    { "DataModel", &DataModel::TYPE },
    { "Snap", &Snap::TYPE },
};