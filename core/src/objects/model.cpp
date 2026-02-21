#include "model.h"
#include "objectmodel/type.h"

INSTANCE_IMPL(Model)

InstanceType Model::__buildType() {
    return make_instance_type<Model>("Model", set_explorer_icon("model"));
}

Model::~Model() = default;