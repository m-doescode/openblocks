#include "script.h"
#include "objects/base/instance.h"

const InstanceType Script::TYPE = {
    .super = &Instance::TYPE,
    .className = "Script",
    .constructor = &Script::Create,
    .explorerIcon = "script",
};

const InstanceType* Script::GetClass() {
    return &TYPE;
}

Script::Script(): Instance(&TYPE) {
}

Script::~Script() {
}