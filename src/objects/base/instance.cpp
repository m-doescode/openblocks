#include "instance.h"
#include "../../common.h"
#include "../../datatypes/meta.h"
#include "datatypes/base.h"
#include "objects/base/member.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <optional>

// Static so that this variable name is "local" to this source file
const InstanceType Instance::TYPE = {
    .super = NULL,
    .className = "Instance",
    .constructor = NULL, // Instance is abstract and therefore not creatable
    .explorerIcon = "instance",
};

// Instance is abstract, so it should not implement GetClass directly
// InstanceType* Instance::GetClass() {
//     return &TYPE_;
// }

Instance::Instance(const InstanceType* type) {
    this->name = type->className;

    this->memberMap = std::make_unique<MemberMap>( MemberMap {
        .super = std::nullopt,
        .members = {
            { "Name", { .backingField = &name, .type = &Data::String::TYPE, .codec = fieldCodecOf<Data::String, std::string>() } }
        }
    });
}

Instance::~Instance () {
}

// TODO: Test this
bool Instance::ancestryContinuityCheck(std::optional<std::shared_ptr<Instance>> newParent) {
    for (std::optional<std::shared_ptr<Instance>> currentParent = newParent; currentParent.has_value(); currentParent = currentParent.value()->GetParent()) {
        if (currentParent.value() == this->shared_from_this())
            return false;
    }
    return true;
}

bool Instance::SetParent(std::optional<std::shared_ptr<Instance>> newParent) {
    if (this->parentLocked || !ancestryContinuityCheck(newParent))
        return false;

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
    return true;
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

tl::expected<Data::Variant, MemberNotFound> Instance::GetPropertyValue(std::string name) {
auto meta = GetPropertyMeta(name);
    if (!meta) return tl::make_unexpected(MemberNotFound());

    return meta->codec.read(meta->backingField);
}

tl::expected<void, MemberNotFound> Instance::SetPropertyValue(std::string name, Data::Variant value) {
    auto meta = GetPropertyMeta(name);
    if (!meta) return tl::make_unexpected(MemberNotFound());

    meta->codec.write(value, meta->backingField);
    if (meta->updateCallback) meta->updateCallback.value()(name);

    return {};
}

tl::expected<PropertyMeta, MemberNotFound> Instance::GetPropertyMeta(std::string name) {
    MemberMap* current = &*memberMap;
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

    MemberMap* current = &*memberMap;
    do {
        for (auto const& [key, _] : current->members) {
            // Don't add it if it's already in the list
            if (std::find(memberList.begin(), memberList.end(), key) == memberList.end())
                memberList.push_back(key);
        }
        if (!current->super.has_value())
            break;
        current = &*current->super.value();
    } while (true);

    cachedMemberList = memberList;
    return memberList;
}