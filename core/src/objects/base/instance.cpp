#include "instance.h"
#include "common.h"
#include "datatypes/meta.h"
#include "datatypes/base.h"
#include "objects/base/member.h"
#include "objects/meta.h"
#include "logger.h"
#include "panic.h"
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

template <typename T>
bool operator ==(std::optional<std::weak_ptr<T>> a, std::optional<std::weak_ptr<T>> b) {
    return (!a.has_value() || a.value().expired()) && (!b.has_value() || b.value().expired())
    || (a.has_value() && !a.value().expired()) && (b.has_value() && !b.value().expired()) && a.value().lock() == b.value().lock();
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

    auto oldDataModel = _dataModel;
    auto oldWorkspace = _workspace;

    // Update parent data model and workspace, if applicable
    if (newParent) {
        this->_dataModel = newParent->get()->GetClass() == &DataModel::TYPE ? std::make_optional(std::dynamic_pointer_cast<DataModel>(newParent.value())) : newParent.value()->dataModel();
        this->_workspace = newParent->get()->GetClass() == &Workspace::TYPE ? std::make_optional(std::dynamic_pointer_cast<Workspace>(newParent.value())) : newParent.value()->workspace();
    } else {
        this->_dataModel = std::nullopt;
        this->_workspace = std::nullopt;
    }

    updateAncestry(this->shared<Instance>(), newParent);

    return true;
}

void Instance::updateAncestry(std::optional<std::shared_ptr<Instance>> updatedChild, std::optional<std::shared_ptr<Instance>> newParent) {
    if (GetParent()) {
        this->_dataModel = GetParent().value()->GetClass() == &DataModel::TYPE ? std::make_optional(std::dynamic_pointer_cast<DataModel>(GetParent().value())) : GetParent().value()->dataModel();
        this->_workspace = GetParent().value()->GetClass() == &Workspace::TYPE ? std::make_optional(std::dynamic_pointer_cast<Workspace>(GetParent().value())) : GetParent().value()->workspace();
    } else {
        this->_dataModel = std::nullopt;
        this->_workspace = std::nullopt;
    }

    OnAncestryChanged(updatedChild, newParent);

    // Update ancestry in descendants
    for (InstanceRef child : children) {
        child->updateAncestry(updatedChild, newParent);
    }
}

std::optional<std::shared_ptr<DataModel>> Instance::dataModel() {
    return (!_dataModel || _dataModel->expired()) ? std::nullopt : std::make_optional(_dataModel.value().lock());
}

std::optional<std::shared_ptr<Workspace>> Instance::workspace() {
    return (!_workspace || _workspace->expired()) ? std::nullopt : std::make_optional(_workspace.value().lock());
}

std::optional<std::shared_ptr<Instance>> Instance::GetParent() {
    if (!parent.has_value()) return std::nullopt;
    if (parent.value().expired()) return std::nullopt;
    return parent.value().lock();
}

static std::shared_ptr<Instance> DUMMY_INSTANCE;
DescendantsIterator Instance::GetDescendantsStart() {
    return DescendantsIterator(GetChildren().size() > 0 ? GetChildren()[0] : DUMMY_INSTANCE);
}

DescendantsIterator Instance::GetDescendantsEnd() {
    return DescendantsIterator(DUMMY_INSTANCE);
}

bool Instance::IsParentLocked() {
    return this->parentLocked;
}

void Instance::OnParentUpdated(std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent) {
    // Empty stub
}

void Instance::OnAncestryChanged(std::optional<std::shared_ptr<Instance>> child, std::optional<std::shared_ptr<Instance>> newParent) {
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

// Serialization

void Instance::Serialize(pugi::xml_node parent) {
    pugi::xml_node node = parent.append_child("Item");
    node.append_attribute("class").set_value(this->GetClass()->className);

    // Add properties
    pugi::xml_node propertiesNode = node.append_child("Properties");
    for (std::string name : GetProperties()) {
        PropertyMeta meta = GetPropertyMeta(name).value();
        if (meta.flags & PropertyFlags::PROP_NOSAVE) continue; // This property should not be serialized. Skip...

        pugi::xml_node propertyNode = propertiesNode.append_child(meta.type->name);
        propertyNode.append_attribute("name").set_value(name);
        GetPropertyValue(name)->Serialize(propertyNode);
    }

    // Add children
    for (InstanceRef child : this->children) {
        child->Serialize(node);
    }
}

result<InstanceRef, NoSuchInstance> Instance::Deserialize(pugi::xml_node node) {
    std::string className = node.attribute("class").value();
    if (INSTANCE_MAP.count(className) == 0) {
        return std::variant<NoSuchInstance>(NoSuchInstance(className));
    }
    // This will error if an abstract instance is used in the file. Oh well, not my prob rn.
    // printf("What are you? A %s sandwich\n", className.c_str());
    InstanceRef object = INSTANCE_MAP[className]->constructor();
    object->GetChildren();

    // const InstanceType* type = INSTANCE_MAP.at(className);

    // Read properties
    pugi::xml_node propertiesNode = node.child("Properties");
    for (pugi::xml_node propertyNode : propertiesNode) {
        std::string propertyName = propertyNode.attribute("name").value();
        auto meta_ = object->GetPropertyMeta(propertyName);
        if (!meta_.has_value()) {
            Logger::fatalErrorf("Attempt to set unknown property '%s' of %s", propertyName.c_str(), object->GetClass()->className.c_str());
            continue;
        }
        Data::Variant value = Data::Variant::Deserialize(propertyNode);
        object->SetPropertyValue(propertyName, value);
    }

    // Read children
    for (pugi::xml_node childNode : node.children("Item")) {
        result<InstanceRef, NoSuchInstance> child = Instance::Deserialize(childNode);
        if (child.is_error()) {
            std::get<NoSuchInstance>(child.error().value()).logMessage();
            continue;
        }
        object->AddChild(child.expect());
    }

    return object;
}

// DescendantsIterator

DescendantsIterator::DescendantsIterator(std::shared_ptr<Instance> current) : current(current), root(current == DUMMY_INSTANCE ? DUMMY_INSTANCE : current->GetParent()), siblingIndex { 0 } { }

DescendantsIterator::self_type DescendantsIterator::operator++(int _) {
    // If the current item is dummy, an error has occurred, this is not supposed to happen.
    if (current == DUMMY_INSTANCE) {
        Logger::fatalError("Attempt to increment a descendant iterator past its end\n");
        panic();
    }

    // If the current item has children, enter it
    if (current->GetChildren().size() > 0) {
        siblingIndex.push_back(0);
        current = current->GetChildren()[0];
        return *this;
    }

    // Otherwise, we move to the next sibling, if applicable.

    // But not if one up is null or the root element
    if (!current->GetParent() || current == root) {
        current = DUMMY_INSTANCE;
        return *this;
    }

    // If we've hit the end of this item's children, move one up
    while (current->GetParent() && current->GetParent().value()->GetChildren().size() <= (siblingIndex.back() + 1)) {
        siblingIndex.pop_back();
        current = current->GetParent().value();

        // But not if one up is null or the root element
        if (!current->GetParent() || current == root) {
            current = DUMMY_INSTANCE;
            return *this;
        }
    }

    // Now move to the next sibling
    siblingIndex.back()++;
    current = current->GetParent().value()->GetChildren()[siblingIndex.back()];

    return *this;
}