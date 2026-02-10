#include "folder.h"

InstanceType Folder::__buildType() {
    return make_instance_type<Folder>("Folder");
}

Folder::~Folder() = default;