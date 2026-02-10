#include "model.h"

InstanceType Model::__buildType() {
    return make_instance_type<Model>("Model");
}

Model::~Model() = default;