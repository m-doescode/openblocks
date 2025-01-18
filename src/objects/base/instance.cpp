#include "instance.h"
#include "../../common.h"
#include <algorithm>
#include <memory>
#include <optional>

// Static so that this variable name is "local" to this source file
static InstanceType TYPE_ {
    .super = NULL,
    .className = "Instance",
    .constructor = NULL, // Instance is abstract and therefore not creatable
};

InstanceType* Instance::TYPE = &TYPE_;

// Instance is abstract, so it should not implement GetClass directly
// InstanceType* Instance::GetClass() {
//     return &TYPE_;
// }

Instance::Instance(InstanceType* type) {
    this->name = type->className;
}

Instance::~Instance () {
}

void Instance::SetParent(std::optional<std::shared_ptr<Instance>> newParent) {
    auto lastParent = GetParent();
    if (hierarchyPreUpdateHandler.has_value()) hierarchyPreUpdateHandler.value()(this->shared_from_this(), lastParent, newParent);
    // If we currently have a parent, remove ourselves from it before adding ourselves to the new one
    if (this->parent.has_value() && !this->parent.value().expired()) {
        auto oldParent = this->parent.value().lock();
        oldParent->children.erase(std::find(oldParent->children.begin(), oldParent->children.end(), this->shared_from_this()));
    }
    // Add ourselves to the new parent
    if (newParent.has_value()) {
        newParent.value()->children.push_back(this->shared_from_this());
    }
    this->parent = newParent;
    // TODO: Add code for sending signals for parent updates
    // TODO: Yeahhh maybe this isn't the best way of doing this?
    if (hierarchyPostUpdateHandler.has_value()) hierarchyPostUpdateHandler.value()(this->shared_from_this(), lastParent, newParent);
}

std::optional<std::shared_ptr<Instance>> Instance::GetParent() {
    if (!parent.has_value()) return std::nullopt;
    if (parent.value().expired()) return std::nullopt;
    return parent.value().lock();
}