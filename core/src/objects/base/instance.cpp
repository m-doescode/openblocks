#include "instance.h"
#include "common.h"
#include "datatypes/primitives.h"
#include "datatypes/variant.h"
#include "datatypes/base.h"
#include "datatypes/ref.h"
#include "error/instance.h"
#include "objects/base/member.h"
#include "objects/base/refstate.h"
#include "objects/datamodel.h"
#include "objects/meta.h"
#include "logger.h"
#include "panic.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <pugixml.hpp>
#include "ptr_helpers.h"

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
}

Instance::~Instance () {
}

template <typename T>
std::weak_ptr<T> optional_to_weak(std::optional<std::shared_ptr<T>> a) {
    return a ? a.value() : std::weak_ptr<T>();
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
    if (!this->parent.expired()) {
        auto oldParent = this->parent.lock();
        oldParent->children.erase(std::find(oldParent->children.begin(), oldParent->children.end(), this->shared_from_this()));
    }
    // Add ourselves to the new parent
    if (newParent.has_value()) {
        newParent.value()->children.push_back(this->shared_from_this());
    }
    this->parent = optional_to_weak(newParent);
    // TODO: Add code for sending signals for parent updates
    // TODO: Yeahhh maybe this isn't the best way of doing this?
    if (hierarchyPostUpdateHandler.has_value()) hierarchyPostUpdateHandler.value()(this->shared_from_this(), lastParent, newParent);

    this->OnParentUpdated(lastParent, newParent);

    updateAncestry(this->shared<Instance>(), newParent);

    return true;
}

void Instance::updateAncestry(std::optional<std::shared_ptr<Instance>> updatedChild, std::optional<std::shared_ptr<Instance>> newParent) {
    auto oldDataModel = _dataModel;
    auto oldWorkspace = _workspace;

    // Update parent data model and workspace, if applicable
    if (GetParent()) {
        this->_dataModel = GetParent().value()->GetClass() == &DataModel::TYPE ? std::dynamic_pointer_cast<DataModel>(GetParent().value()) : GetParent().value()->_dataModel;
        this->_workspace = GetParent().value()->GetClass() == &Workspace::TYPE ? std::dynamic_pointer_cast<Workspace>(GetParent().value()) : GetParent().value()->_workspace;
    } else {
        this->_dataModel = {};
        this->_workspace = {};
    }

    OnAncestryChanged(updatedChild, newParent);
    AncestryChanged->Fire({updatedChild.has_value() ? InstanceRef(updatedChild.value()) : InstanceRef(), newParent.has_value() ? InstanceRef(newParent.value()) : InstanceRef()});

    // Old workspace used to exist, and workspaces differ
    if (!oldWorkspace.expired() && oldWorkspace != _workspace) {
        OnWorkspaceRemoved(oldWorkspace.lock());
    }

    // New workspace exists, and workspaces differ
    if (!_workspace.expired() && (_workspace != oldWorkspace)) {
        OnWorkspaceAdded(!oldWorkspace.expired() ? std::make_optional(oldWorkspace.lock()) : std::nullopt, _workspace.lock());
    }

    // Update ancestry in descendants
    for (std::shared_ptr<Instance> child : children) {
        child->updateAncestry(updatedChild, newParent);
    }
}

std::optional<std::shared_ptr<DataModel>> Instance::dataModel() {
    return (_dataModel.expired()) ? std::nullopt : std::make_optional(_dataModel.lock());
}

std::optional<std::shared_ptr<Workspace>> Instance::workspace() {
    return (_workspace.expired()) ? std::nullopt : std::make_optional(_workspace.lock());
}

std::optional<std::shared_ptr<Instance>> Instance::GetParent() {
    if (parent.expired()) return std::nullopt;
    return parent.lock();
}

void Instance::Destroy() {
    if (parentLocked) return;
    // TODO: Implement proper distruction stuff
    SetParent(std::nullopt);
    parentLocked = true;
}

bool Instance::IsA(std::string className) {
    const InstanceType* cur = GetClass();
    while (cur && cur->className != className) { cur = cur->super; }
    return cur != nullptr;
}

std::optional<std::shared_ptr<Instance>> Instance::FindFirstChild(std::string name) {
    for (auto child : children) {
        if (child->name == name)
            return child;
    }
    return std::nullopt;
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

void Instance::OnWorkspaceAdded(std::optional<std::shared_ptr<Workspace>> oldWorkspace, std::shared_ptr<Workspace> newWorkspace) {
    // Empty stub
}

void Instance::OnWorkspaceRemoved(std::shared_ptr<Workspace> oldWorkspace) {
    // Empty stub
}

// Properties

result<Variant, MemberNotFound> Instance::GetPropertyValue(std::string name) {
    return InternalGetPropertyValue(name);
}

fallible<MemberNotFound, AssignToReadOnlyMember> Instance::SetPropertyValue(std::string name, Variant value, bool sendUpdateEvent) {
    auto result = InternalSetPropertyValue(name, value);
    if (result.isSuccess() && sendUpdateEvent) {
        InternalUpdateProperty(name);
        sendPropertyUpdatedSignal(shared_from_this(), name, value);
    }
    return result;
}

result<PropertyMeta, MemberNotFound> Instance::GetPropertyMeta(std::string name) {
    return InternalGetPropertyMeta(name);
}


result<Variant, MemberNotFound> Instance::InternalGetPropertyValue(std::string name) {
    if (name == "Name") {
        return Variant(this->name);
    } else if (name == "Parent") {
        return Variant(InstanceRef(this->parent));
    } else if (name == "ClassName") {
        return Variant(GetClass()->className);
    }
    return MemberNotFound(GetClass()->className, name);
}

result<PropertyMeta, MemberNotFound> Instance::InternalGetPropertyMeta(std::string name) {
    if (name == "Name") {
        return PropertyMeta { &STRING_TYPE };
    } else if (name == "Parent") {
        return PropertyMeta { &InstanceRef::TYPE, PROP_NOSAVE };
    } else if (name == "ClassName") {
        return PropertyMeta { &STRING_TYPE, PROP_NOSAVE | PROP_READONLY };
    }
    return MemberNotFound(GetClass()->className, name);
}

fallible<MemberNotFound, AssignToReadOnlyMember> Instance::InternalSetPropertyValue(std::string name, Variant value) {
    if (name == "Name") {
        this->name = (std::string)value.get<std::string>();
    } else if (name == "Parent") {
        std::weak_ptr<Instance> ref = value.get<InstanceRef>();
        SetParent(ref.expired() ? std::nullopt : std::make_optional(ref.lock()));
    } else if (name == "ClassName") {
        return AssignToReadOnlyMember(GetClass()->className, name);
    } else {
        return MemberNotFound(GetClass()->className, name);
    }
    return {};
}

void Instance::InternalUpdateProperty(std::string name) {
}

std::vector<std::string> Instance::InternalGetProperties() {
    std::vector<std::string> members;
    members.push_back("Name");
    members.push_back("Parent");
    members.push_back("ClassName");
    return members;
}

void Instance::UpdateProperty(std::string name) {
    InternalUpdateProperty(name);
}

std::vector<std::string> Instance::GetProperties() {
    if (cachedMemberList.has_value()) return cachedMemberList.value();

    std::vector<std::string> memberList = InternalGetProperties();

    cachedMemberList = memberList;
    return memberList;
}

// Serialization

void Instance::Serialize(pugi::xml_node parent, RefStateSerialize state) {
    pugi::xml_node node = parent.append_child("Item");
    node.append_attribute("class").set_value(this->GetClass()->className);

    // Add properties
    pugi::xml_node propertiesNode = node.append_child("Properties");
    for (std::string name : GetProperties()) {
        PropertyMeta meta = GetPropertyMeta(name).expect("Meta of declared property is missing");
        if (meta.flags & (PROP_NOSAVE | PROP_READONLY)) continue; // This property should not be serialized. Skip...

        pugi::xml_node propertyNode = propertiesNode.append_child(meta.type->name);
        propertyNode.append_attribute("name").set_value(name);

        // Update std::shared_ptr<Instance> properties using map above
        if (meta.type == &InstanceRef::TYPE) {
            std::weak_ptr<Instance> refWeak = GetPropertyValue(name).expect("Declared property is missing").get<InstanceRef>();
            if (refWeak.expired()) continue;

            auto ref = refWeak.lock();
            auto remappedRef = state->remappedInstances[ref]; // TODO: I think this is okay? Maybe?? Add null check?
            
            if (remappedRef != "") {
                // If the instance has already been remapped, set the new value
                propertyNode.text().set(remappedRef);
            } else {
                // Otheriise, queue this property to be updated later, and keep its current value
                auto& refs = state->refsAwaitingRemap[ref];
                refs.push_back(propertyNode);
                state->refsAwaitingRemap[ref] = refs;
            }
        } else {
            GetPropertyValue(name).expect("Declared property is missing").Serialize(propertyNode);
        }
    }

    // Remap self
    std::string remappedId = "OB" + std::to_string(state->count++);
    state->remappedInstances[shared_from_this()] = remappedId;
    node.append_attribute("referent").set_value(remappedId);

    // Remap queued properties
    for (pugi::xml_node ref : state->refsAwaitingRemap[shared_from_this()]) {
        ref.text().set(remappedId);
    }
    state->refsAwaitingRemap[shared_from_this()].clear();

    // Add children
    for (std::shared_ptr<Instance> child : this->children) {
        child->Serialize(node, state);
    }
}

result<std::shared_ptr<Instance>, NoSuchInstance> Instance::Deserialize(pugi::xml_node node, RefStateDeserialize state) {
    std::string className = node.attribute("class").value();
    if (INSTANCE_MAP.count(className) == 0) {
        return NoSuchInstance(className);
    }
    // This will error if an abstract instance is used in the file. Oh well, not my prob rn.
    std::shared_ptr<Instance> object = INSTANCE_MAP[className]->constructor();
    object->GetChildren();

    // const InstanceType* type = INSTANCE_MAP.at(className);

    // Read properties
    pugi::xml_node propertiesNode = node.child("Properties");
    for (pugi::xml_node propertyNode : propertiesNode) {
        std::string propertyName = propertyNode.attribute("name").value();
        auto meta_ = object->GetPropertyMeta(propertyName);
        if (!meta_) {
            Logger::fatalErrorf("Attempt to set unknown property '%s' of %s", propertyName.c_str(), object->GetClass()->className.c_str());
            continue;
        }

        // Update std::shared_ptr<Instance> properties using map above
        if (meta_.expect().type == &InstanceRef::TYPE) {
            if (propertyNode.text().empty())
                continue;

            std::string refId = propertyNode.text().as_string();
            auto remappedRef = state->remappedInstances[refId]; // TODO: I think this is okay? Maybe?? Add null check?
            
            if (remappedRef) {
                // If the instance has already been remapped, set the new value
                object->SetPropertyValue(propertyName, InstanceRef(remappedRef)).expect();
            } else {
                // Otheriise, queue this property to be updated later, and keep its current value
                auto& refs = state->refsAwaitingRemap[refId];
                refs.push_back(std::make_pair(object, propertyName));
                state->refsAwaitingRemap[refId] = refs;

                object->SetPropertyValue(propertyName, InstanceRef()).expect();
            }
        } else {
            Variant value = Variant::Deserialize(propertyNode);
            object->SetPropertyValue(propertyName, value).expect("Declared property was missing");
        }
    }

    // Remap self
    std::string remappedId = node.attribute("referent").value();
    state->remappedInstances[remappedId] = object;

    // Remap queued properties
    for (std::pair<std::shared_ptr<Instance>, std::string> ref : state->refsAwaitingRemap[remappedId]) {
        ref.first->SetPropertyValue(ref.second, InstanceRef(object)).expect();
    }
    state->refsAwaitingRemap[remappedId].clear();

    // Read children
    for (pugi::xml_node childNode : node.children("Item")) {
        result<std::shared_ptr<Instance>, NoSuchInstance> child = Instance::Deserialize(childNode, state);
        if (child.isError()) {
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

std::optional<std::shared_ptr<Instance>> Instance::Clone(RefStateClone state) {
    std::shared_ptr<Instance> newInstance = GetClass()->constructor();

    // Copy properties
    for (std::string property : GetProperties()) {
        PropertyMeta meta = GetPropertyMeta(property).expect();
        
        if (meta.flags & (PROP_READONLY | PROP_NOSAVE)) continue;

        // Update std::shared_ptr<Instance> properties using map above
        if (meta.type == &InstanceRef::TYPE) {
            std::weak_ptr<Instance> refWeak = GetPropertyValue(property).expect().get<InstanceRef>();
            if (refWeak.expired()) continue;

            auto ref = refWeak.lock();
            auto remappedRef = state->remappedInstances[ref]; // TODO: I think this is okay? Maybe?? Add null check?
            
            if (remappedRef) {
                // If the instance has already been remapped, set the new value
                newInstance->SetPropertyValue(property, InstanceRef(remappedRef)).expect();
            } else {
                // Otheriise, queue this property to be updated later, and keep its current value
                auto& refs = state->refsAwaitingRemap[ref];
                refs.push_back(std::make_pair(newInstance, property));
                state->refsAwaitingRemap[ref] = refs;

                newInstance->SetPropertyValue(property, InstanceRef(ref)).expect();
            }
        } else {
            Variant value = GetPropertyValue(property).expect();
            newInstance->SetPropertyValue(property, value).expect();
        }
    }

    // Remap self
    state->remappedInstances[shared_from_this()] = newInstance;

    // Remap queued properties
    for (std::pair<std::shared_ptr<Instance>, std::string> ref : state->refsAwaitingRemap[shared_from_this()]) {
        ref.first->SetPropertyValue(ref.second, InstanceRef(newInstance)).expect();
    }
    state->refsAwaitingRemap[shared_from_this()].clear();

    // Clone children
    for (std::shared_ptr<Instance> child : GetChildren()) {
        std::optional<std::shared_ptr<Instance>> clonedChild = child->Clone(state);
        if (clonedChild)
            newInstance->AddChild(clonedChild.value());
    }

    return newInstance;
}

std::vector<std::pair<std::string, std::shared_ptr<Instance>>> Instance::GetReferenceProperties() {
    std::vector<std::pair<std::string, std::shared_ptr<Instance>>> referenceProperties;

    auto propertyNames = GetProperties();

    for (std::string property : propertyNames) {
        PropertyMeta meta = GetPropertyMeta(property).expect();
        if (meta.type != &InstanceRef::TYPE) continue;

        std::weak_ptr<Instance> ref = GetPropertyValue(property).expect().get<InstanceRef>();
        if (ref.expired()) continue;
        referenceProperties.push_back(std::make_pair(property, ref.lock()));
    }

    return referenceProperties;
}

std::string Instance::GetFullName() {
    std::string currentName = name;
    std::optional<std::shared_ptr<Instance>> currentParent = GetParent();

    while (currentParent.has_value() && !currentParent.value()->IsA("DataModel")) {
        currentName = currentParent.value()->name + "." + currentName;

        currentParent = currentParent.value()->GetParent();
    }

    return currentName;
}