#pragma once

#include <string>
#include <map>
#include "objects/base/instance.h"

// Map of all instance types to their class names
extern std::map<std::string, const InstanceType*> INSTANCE_MAP;