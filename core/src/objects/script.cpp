#include "script.h"
#include "objects/base/instance.h"
#include "objects/base/member.h"

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
    this->memberMap = std::make_unique<MemberMap>(MemberMap {
        .super = std::move(this->memberMap),
        .members = {
            { "Source", {
                .backingField = &source,
                .type = &Data::String::TYPE,
                .codec = fieldCodecOf<Data::String, std::string>(),
                .flags = PROP_HIDDEN,
            }},
        }
    });
}

Script::~Script() {
}