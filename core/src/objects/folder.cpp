#include "folder.h"
#include "objectmodel/type.h"

InstanceType Folder::__buildType() {
    return make_instance_type<Folder>("Folder", set_explorer_icon("folder"));
}

Folder::~Folder() = default;