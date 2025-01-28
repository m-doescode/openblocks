#include "instance.h"
#include "../../common.h"
#include "objects/base/member.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <optional>

// Static so that this variable name is "local" to this source file
static InstanceType TYPE_ {
    .super = NULL,
    .className = "Instance",
    .constructor = NULL, // Instance is abstract and therefore not creatable
    .explorerIcon = "instance",
};

InstanceType* Instance::TYPE = &TYPE_;

// Instance is abstract, so it should not implement GetClass directly
// InstanceType* Instance::GetClass() {
//     return &TYPE_;
// }

Instance::Instance(InstanceType* type) {
    this->name = type->className;

    this->memberMap = {
        .super = std::nullopt,
        .members = {
            { "Name", { .backingField = &name } }
        }
    };
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

    this->OnParentUpdated(lastParent, newParent);
}

std::optional<std::shared_ptr<Instance>> Instance::GetParent() {
    if (!parent.has_value()) return std::nullopt;
    if (parent.value().expired()) return std::nullopt;
    return parent.value().lock();
}


void Instance::OnParentUpdated(std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent) {
    // Empty stub
}

// Properties

tl::expected<std::string, MemberNotFound> Instance::GetPropertyValue(std::string name) {
auto meta = GetPropertyMeta(name);
    if (!meta) return tl::make_unexpected(MemberNotFound());

    return *(std::string*)meta->backingField;
}

tl::expected<void, MemberNotFound> Instance::SetPropertyValue(std::string name, std::string value) {
    auto meta = GetPropertyMeta(name);
    if (!meta) return tl::make_unexpected(MemberNotFound());

    *(std::string*)meta->backingField = value;

    return {};
}

tl::expected<PropertyMeta, MemberNotFound> Instance::GetPropertyMeta(std::string name) {
    MemberMap* current = &memberMap;
    while (true) {
        // We look for the property in current member map
        auto it = current->members.find(name);

        // It is found, return it
        if (it != current->members.end())
            return it->second;

        // It is not found, If there are no other maps to search, return null
        if (!current->super.has_value())
            return tl::make_unexpected(MemberNotFound());

        // Search in the parent
        current = current->super->get();
    }
}

std::vector<std::string> Instance::GetProperties() {
    if (cachedMemberList.has_value()) return cachedMemberList.value();

    std::vector<std::string> memberList;

    MemberMap* current = &memberMap;
    do {
        for (auto const& [key, _] : current->members) {
            // Don't add it if it's already in the list
            if (std::find(memberList.begin(), memberList.end(), key) == memberList.end())
                memberList.push_back(key);
        }
    } while (current->super.has_value());

    cachedMemberList = memberList;
    return memberList;
}