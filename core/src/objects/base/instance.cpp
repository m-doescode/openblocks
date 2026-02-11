#include "instance.h"
#include "common.h"
#include "datatypes/primitives.h"
#include "datatypes/variant.h"
#include "datatypes/base.h"
#include "datatypes/ref.h"
#include "error/instance.h"
#include "objectmodel/property.h"
#include "objectmodel/type.h"
#include "objects/base/member.h"
#include "objects/base/refstate.h"
#include "objects/datamodel.h"
#include "objects/meta.h"
#include "logger.h"
#include "panic.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <pugixml.hpp>
#include "ptr_helpers.h" // IWYU pragma: keep

// TODO: Maybe find a better solution?
// Because def_property refers to Instance::Type(), we have to feed it this way instead
template <typename T, typename C>
InstanceProperty def_property_apex(const InstanceType& typeMeta, std::string name, T C::* ref, PropertyFlags flags = 0, PropertyListener listener = {}) {
    return {
        name,
        &typeMeta,
        flags,
        "",

        [ref](std::shared_ptr<Instance> instance) {
            auto obj = std::dynamic_pointer_cast<C>(instance);
            return obj.get()->*ref;
        },
        [ref](std::shared_ptr<Instance> instance, Variant value) {
            auto obj = std::dynamic_pointer_cast<C>(instance);
            obj.get()->*ref = value.get<T>();
        },
        listener
    };
}

InstanceType __init() {
    InstanceType type;
    type.className = "<NULL>";
    return type;
}

const InstanceType& Instance::Type() {
    static InstanceType type = __init();
    if (type.className == "<NULL>") {
        type.className = "Instance";
        type.super = nullptr;
        type.flags = INSTANCE_NOTCREATABLE;
        type.explorerIcon = "instance";

        type.properties["Name"] = def_property("Name", &Instance::name);
        type.properties["Parent"] = def_property_apex(type, "Parent", &Instance::parent, PROP_NOSAVE);
        type.properties["ClassName"] = def_property<std::string, Instance>("ClassName", [](Instance* obj){ return obj->GetType().className; }, PROP_NOSAVE | PROP_READONLY);
    }

    return type;
}

Instance::~Instance () {
}

// TODO: Test this
bool Instance::ancestryContinuityCheck(nullable std::shared_ptr<Instance> newParent) {
    for (std::shared_ptr<Instance> currentParent = newParent; currentParent != nullptr; currentParent = currentParent->GetParent()) {
        if (currentParent == this->shared_from_this())
            return false;
    }
    return true;
}

bool Instance::SetParent(nullable std::shared_ptr<Instance> newParent) {
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
    if (newParent != nullptr) {
        newParent->children.push_back(this->shared_from_this());
    }
    this->parent = newParent;
    // TODO: Add code for sending signals for parent updates
    // TODO: Yeahhh maybe this isn't the best way of doing this?
    if (hierarchyPostUpdateHandler.has_value()) hierarchyPostUpdateHandler.value()(this->shared_from_this(), lastParent, newParent);

    this->OnParentUpdated(lastParent, newParent);

    updateAncestry(this->shared<Instance>(), newParent);

    return true;
}

void Instance::updateAncestry(nullable std::shared_ptr<Instance> updatedChild, nullable std::shared_ptr<Instance> newParent) {
    auto oldDataModel = _dataModel;
    auto oldWorkspace = _workspace;

    // Update parent data model and workspace, if applicable
    if (GetParent() != nullptr) {
        this->_dataModel = GetParent()->GetType() == DataModel::Type() ? std::dynamic_pointer_cast<DataModel>(GetParent()) : GetParent()->_dataModel;
        this->_workspace = GetParent()->GetType() == Workspace::Type() ? std::dynamic_pointer_cast<Workspace>(GetParent()) : GetParent()->_workspace;
    } else {
        this->_dataModel = {};
        this->_workspace = {};
    }

    OnAncestryChanged(updatedChild, newParent);
    AncestryChanged->Fire({updatedChild != nullptr ? InstanceRef(updatedChild) : InstanceRef(), newParent != nullptr ? InstanceRef(newParent) : InstanceRef()});

    // Old workspace used to exist, and workspaces differ
    if (!oldWorkspace.expired() && oldWorkspace != _workspace) {
        OnWorkspaceRemoved(oldWorkspace.lock());
    }

    // New workspace exists, and workspaces differ
    if (!_workspace.expired() && (_workspace != oldWorkspace)) {
        OnWorkspaceAdded(oldWorkspace.expired() ? nullptr : oldWorkspace.lock(), _workspace.lock());
    }

    // Update ancestry in descendants
    for (std::shared_ptr<Instance> child : children) {
        child->updateAncestry(updatedChild, newParent);
    }
}

nullable std::shared_ptr<DataModel> Instance::dataModel() {
    return _dataModel.expired() ? nullptr : _dataModel.lock();
}

nullable std::shared_ptr<Workspace> Instance::workspace() {
    return _workspace.expired() ? nullptr : _workspace.lock();
}

nullable std::shared_ptr<Instance> Instance::GetParent() {
    return parent.expired() ? nullptr : parent.lock();
}

void Instance::Destroy() {
    if (parentLocked) return;
    // TODO: Implement proper distruction stuff
    SetParent(nullptr);
    parentLocked = true;
}

bool Instance::IsA(std::string className) {
    const InstanceType* cur = &GetType();
    while (cur && cur->className != className) { cur = cur->super; }
    return cur != nullptr;
}

nullable std::shared_ptr<Instance> Instance::FindFirstChild(std::string name) {
    for (auto child : children) {
        if (child->name == name)
            return child;
    }
    return nullptr;
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

void Instance::OnParentUpdated(nullable std::shared_ptr<Instance> oldParent, nullable std::shared_ptr<Instance> newParent) {
    // Empty stub
}

void Instance::OnAncestryChanged(nullable std::shared_ptr<Instance> child, nullable std::shared_ptr<Instance> newParent) {
    // Empty stub
}

void Instance::OnWorkspaceAdded(nullable std::shared_ptr<Workspace> oldWorkspace, std::shared_ptr<Workspace> newWorkspace) {
    // Empty stub
}

void Instance::OnWorkspaceRemoved(std::shared_ptr<Workspace> oldWorkspace) {
    // Empty stub
}

// Properties

result<Variant, MemberNotFound> Instance::GetProperty(std::string name) {
    name[0] = toupper(name[0]); // Ignore case of first character
    return InternalGetPropertyValue(name);
}

fallible<MemberNotFound, AssignToReadOnlyMember> Instance::SetProperty(std::string name, Variant value, bool sendUpdateEvent) {
    name[0] = toupper(name[0]); // Ignore case of first character
    auto result = InternalSetPropertyValue(name, value);
    if (result.isSuccess() && sendUpdateEvent) {
        InternalUpdateProperty(name);
        sendPropertyUpdatedSignal(shared_from_this(), name, value);
    }
    return result;
}

result<PropertyMeta, MemberNotFound> Instance::GetPropertyMeta(std::string name) {
    name[0] = toupper(name[0]); // Ignore case of first character
    return InternalGetPropertyMeta(name);
}


result<Variant, MemberNotFound> Instance::InternalGetPropertyValue(std::string name) {
    auto& type = GetType();
    if (type.properties.count(name) == 0) {
        return MemberNotFound(GetType().className, name);
    }
    return type.properties.at(name).getter(shared_from_this());
}

result<PropertyMeta, MemberNotFound> Instance::InternalGetPropertyMeta(std::string name) {
    auto& type = GetType();
    if (type.properties.count(name) == 0) {
        return MemberNotFound(GetType().className, name);
    }
    auto& property = type.properties.at(name);

    return { property.type, property.flags, property.category };
}

fallible<MemberNotFound, AssignToReadOnlyMember> Instance::InternalSetPropertyValue(std::string name, Variant value) {
    auto& type = GetType();
    if (type.properties.count(name) == 0) {
        return MemberNotFound(GetType().className, name);
    }
    auto& property = type.properties.at(name);

    if (property.flags & PROP_READONLY) {
        return AssignToReadOnlyMember(GetType().className, name);
    }

    property.setter(shared_from_this(), value);
    return {};
}

void Instance::InternalUpdateProperty(std::string name) {
}

std::vector<std::string> Instance::InternalGetProperties() {
    std::vector<std::string> members;
    for (auto& [key, value] : GetType().properties) {
        members.push_back(key);
    }
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
    if (state == nullptr) state = std::make_shared<__RefStateSerialize>();
    pugi::xml_node node = parent.append_child("Item");
    node.append_attribute("class").set_value(this->GetType().className);

    // Add properties
    pugi::xml_node propertiesNode = node.append_child("Properties");
    for (std::string name : GetProperties()) {
        PropertyMeta meta = GetPropertyMeta(name).expect("Meta of declared property is missing");
        if (meta.flags & (PROP_NOSAVE | PROP_READONLY)) continue; // This property should not be serialized. Skip...

#if 1
        // Special consideration for Part.Size
        // It should be serialized as "size" to be compatible with rbxl files
        // This is optional, as they can still be opened otherwise, but I opted
        // to keep this enabled
        if (IsA("Part") && name == "Size")
            name = "size";
#endif

        pugi::xml_node propertyNode = propertiesNode.append_child(meta.type.descriptor->name);
        propertyNode.append_attribute("name").set_value(name);

        // Update std::shared_ptr<Instance> properties using map above
        if (meta.type.descriptor == &InstanceRef::TYPE) {
            std::weak_ptr<Instance> refWeak = GetProperty(name).expect("Declared property is missing").get<InstanceRef>();
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
            GetProperty(name).expect("Declared property is missing").Serialize(propertyNode);
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
    if (state == nullptr) state = std::make_shared<__RefStateDeserialize>();
    std::string className = node.attribute("class").value();
    if (INSTANCE_MAP.count(className) == 0) {
        return NoSuchInstance(className);
    }
    
    std::optional<InstanceConstructor> constructor = INSTANCE_MAP[className]->constructor;
    if (!constructor.has_value()) {
        // TODO: Replace this with a more appropriate error
        return NoSuchInstance("TODO: <constructor for: " + className + ">");
    }

    std::shared_ptr<Instance> object = constructor.value()();
    object->GetChildren();

    // const InstanceType* type = INSTANCE_MAP.at(className);

    // Read properties
    pugi::xml_node propertiesNode = node.child("Properties");
    for (pugi::xml_node propertyNode : propertiesNode) {
        std::string propertyName = propertyNode.attribute("name").value();
        auto meta_ = object->GetPropertyMeta(propertyName);
        if (!meta_) {
            Logger::fatalErrorf("Attempt to set unknown property '%s' of %s", propertyName.c_str(), object->GetType().className.c_str());
            continue;
        }
        auto meta = meta_.expect();

        // Update std::shared_ptr<Instance> properties using map above
        if (meta.type.descriptor == &InstanceRef::TYPE) {
            if (propertyNode.text().empty())
                continue;

            std::string refId = propertyNode.text().as_string();
            auto remappedRef = state->remappedInstances[refId]; // TODO: I think this is okay? Maybe?? Add null check?
            
            if (remappedRef) {
                // If the instance has already been remapped, set the new value
                object->SetProperty(propertyName, InstanceRef(remappedRef)).expect();
            } else {
                // Otheriise, queue this property to be updated later, and keep its current value
                auto& refs = state->refsAwaitingRemap[refId];
                refs.push_back(std::make_pair(object, propertyName));
                state->refsAwaitingRemap[refId] = refs;

                object->SetProperty(propertyName, InstanceRef()).expect();
            }
        } else {
            auto valueResult = Variant::Deserialize(propertyNode, meta.type);
            if (valueResult.isError()) {
                valueResult.logError();
                continue;
            }
            auto value = valueResult.expect();
            object->SetProperty(propertyName, value).expect("Declared property was missing");
        }
    }

    // Remap self
    std::string remappedId = node.attribute("referent").value();
    state->remappedInstances[remappedId] = object;

    // Remap queued properties
    for (std::pair<std::shared_ptr<Instance>, std::string> ref : state->refsAwaitingRemap[remappedId]) {
        ref.first->SetProperty(ref.second, InstanceRef(object)).expect();
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

DescendantsIterator::DescendantsIterator(std::shared_ptr<Instance> current) : root(current == DUMMY_INSTANCE ? DUMMY_INSTANCE : current->GetParent()), current(current), siblingIndex { 0 } { }

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
    while (current->GetParent() != nullptr && current->GetParent()->GetChildren().size() <= size_t(siblingIndex.back() + 1)) {
        siblingIndex.pop_back();
        current = current->GetParent();

        // But not if one up is null or the root element
        if (!current->GetParent() || current == root) {
            current = DUMMY_INSTANCE;
            return *this;
        }
    }

    // Now move to the next sibling
    siblingIndex.back()++;
    current = current->GetParent()->GetChildren()[siblingIndex.back()];

    return *this;
}

nullable std::shared_ptr<Instance> Instance::Clone(RefStateClone state) {
    if (state == nullptr) state = std::make_shared<__RefStateClone>();
    // TODO: Handle case where this is NotCreatable
    std::shared_ptr<Instance> newInstance = GetType().constructor.value()();

    // Copy properties
    for (std::string property : GetProperties()) {
        PropertyMeta meta = GetPropertyMeta(property).expect();
        
        if (meta.flags & (PROP_READONLY | PROP_NOSAVE)) continue;

        // Update std::shared_ptr<Instance> properties using map above
        if (meta.type.descriptor == &InstanceRef::TYPE) {
            std::weak_ptr<Instance> refWeak = GetProperty(property).expect().get<InstanceRef>();
            if (refWeak.expired()) continue;

            auto ref = refWeak.lock();
            auto remappedRef = state->remappedInstances[ref]; // TODO: I think this is okay? Maybe?? Add null check?
            
            if (remappedRef) {
                // If the instance has already been remapped, set the new value
                newInstance->SetProperty(property, InstanceRef(remappedRef)).expect();
            } else {
                // Otheriise, queue this property to be updated later, and keep its current value
                auto& refs = state->refsAwaitingRemap[ref];
                refs.push_back(std::make_pair(newInstance, property));
                state->refsAwaitingRemap[ref] = refs;

                newInstance->SetProperty(property, InstanceRef(ref)).expect();
            }
        } else {
            Variant value = GetProperty(property).expect();
            newInstance->SetProperty(property, value).expect();
        }
    }

    // Remap self
    state->remappedInstances[shared_from_this()] = newInstance;

    // Remap queued properties
    for (std::pair<std::shared_ptr<Instance>, std::string> ref : state->refsAwaitingRemap[shared_from_this()]) {
        ref.first->SetProperty(ref.second, InstanceRef(newInstance)).expect();
    }
    state->refsAwaitingRemap[shared_from_this()].clear();

    // Clone children
    for (std::shared_ptr<Instance> child : GetChildren()) {
        nullable std::shared_ptr<Instance> clonedChild = child->Clone(state);
        if (clonedChild)
            newInstance->AddChild(clonedChild);
    }

    return newInstance;
}

std::vector<std::pair<std::string, std::shared_ptr<Instance>>> Instance::GetReferenceProperties() {
    std::vector<std::pair<std::string, std::shared_ptr<Instance>>> referenceProperties;

    auto propertyNames = GetProperties();

    for (std::string property : propertyNames) {
        PropertyMeta meta = GetPropertyMeta(property).expect();
        if (meta.type.descriptor != &InstanceRef::TYPE) continue;

        std::weak_ptr<Instance> ref = GetProperty(property).expect().get<InstanceRef>();
        if (ref.expired()) continue;
        referenceProperties.push_back(std::make_pair(property, ref.lock()));
    }

    return referenceProperties;
}

std::string Instance::GetFullName() {
    std::string currentName = name;
    nullable std::shared_ptr<Instance> currentParent = GetParent();

    while (currentParent && !currentParent->IsA("DataModel")) {
        currentName = currentParent->name + "." + currentName;

        currentParent = currentParent->GetParent();
    }

    return currentName;
}

result<std::shared_ptr<Instance>, NoSuchInstance, NotCreatableInstance> Instance::New(std::string className) {
    const InstanceType* type = INSTANCE_MAP[className];

    if (type == nullptr) {
        return NoSuchInstance(className);
    }
    
    if (type->flags & (INSTANCE_NOTCREATABLE | INSTANCE_SERVICE) || type->constructor == nullptr)
        return NotCreatableInstance(className);
    
    return type->constructor.value()();
}