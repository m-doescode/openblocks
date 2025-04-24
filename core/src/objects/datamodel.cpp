#include "datamodel.h"
#include "base/service.h"
#include "objects/base/instance.h"
#include "objects/base/refstate.h"
#include "objects/base/service.h"
#include "objects/meta.h"
#include "objects/script/serverscriptservice.h"
#include "workspace.h"
#include "logger.h"
#include "panic.h"
#include <cstdio>
#include <fstream>
#include <memory>
#include <optional>

const InstanceType DataModel::TYPE = {
    .super = &Instance::TYPE,
    .className = "DataModel",
    .constructor = nullptr,
};

const InstanceType* DataModel::GetClass() {
    return &TYPE;
}

DataModel::DataModel()
    : Instance(&TYPE) {
    this->name = "Place";
}

void DataModel::Init(bool runMode) {
    // Create the workspace if it doesn't exist
    if (this->services.count("Workspace") == 0) {
        this->services["Workspace"] = std::make_shared<Workspace>();
        AddChild(this->services["Workspace"]);
    }

    GetService<ServerScriptService>();
    
    // Init all services
    for (auto [_, service] : this->services) {
        service->InitService();
        if (runMode) service->OnRun();
    }
}

void DataModel::SaveToFile(std::optional<std::string> path) {
    if (!path.has_value() && !this->currentFile.has_value()) {
        Logger::fatalError("Cannot save DataModel because no path was provided.");
        panic();
    }

    std::string target = path.has_value() ? path.value() : this->currentFile.value();

    std::ofstream outStream(target);
    
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("openblocks");

    for (InstanceRef child : this->GetChildren()) {
        child->Serialize(root);
    }

    doc.save(outStream);
    currentFile = target;
    name = target;
    Logger::info("Place saved successfully");
}

void DataModel::DeserializeService(pugi::xml_node node) {
    std::string className = node.attribute("class").value();
    if (INSTANCE_MAP.count(className) == 0) {
        Logger::fatalErrorf("Unknown service: '%s'", className.c_str());
        return;
    }

    if (services.count(className) != 0) {
        Logger::fatalErrorf("Service %s defined multiple times in file", className.c_str());
        return;
    }

    // This will error if an abstract instance is used in the file. Oh well, not my prob rn.
    InstanceRef object = INSTANCE_MAP[className]->constructor();
    AddChild(object);

    // Read properties
    pugi::xml_node propertiesNode = node.child("Properties");
    for (pugi::xml_node propertyNode : propertiesNode) {
        std::string propertyName = propertyNode.attribute("name").value();
        auto meta_ = object->GetPropertyMeta(propertyName);
        if (!meta_) {
            Logger::fatalErrorf("Attempt to set unknown property '%s' of %s", propertyName.c_str(), object->GetClass()->className.c_str());
            continue;
        }
        Data::Variant value = Data::Variant::Deserialize(propertyNode);
        object->SetPropertyValue(propertyName, value).expect();
    }

    // Add children
    for (pugi::xml_node childNode : node.children("Item")) {
        result<InstanceRef, NoSuchInstance> child = Instance::Deserialize(childNode);
        if (child.isError()) {
            std::get<NoSuchInstance>(child.error().value()).logMessage();
            continue;
        }
        object->AddChild(child.expect());
    }

    // We add the service to the list
    // All services get init'd at once in InitServices
    this->services[className] = std::dynamic_pointer_cast<Service>(object);
}

std::shared_ptr<DataModel> DataModel::LoadFromFile(std::string path) {
    std::ifstream inStream(path);
    pugi::xml_document doc;
    doc.load(inStream);

    pugi::xml_node rootNode = doc.child("openblocks");
    std::shared_ptr<DataModel> newModel = std::make_shared<DataModel>();

    for (pugi::xml_node childNode : rootNode.children("Item")) {
        newModel->DeserializeService(childNode);
    }

    newModel->Init();

    return newModel;
}

result<std::shared_ptr<Service>, NoSuchService> DataModel::GetService(std::string className) {
    if (services.count(className) != 0)
        return std::dynamic_pointer_cast<Service>(services[className]);
    
    if (!INSTANCE_MAP[className] || ~(INSTANCE_MAP[className]->flags & (INSTANCE_NOTCREATABLE | INSTANCE_SERVICE)) == 0) {
        return NoSuchService(className);
    }

    services[className] = std::dynamic_pointer_cast<Service>(INSTANCE_MAP[className]->constructor());
    AddChild(std::dynamic_pointer_cast<Instance>(services[className]));

    return std::dynamic_pointer_cast<Service>(services[className]);
}

result<std::optional<std::shared_ptr<Service>>, NoSuchService> DataModel::FindService(std::string className) {
    if (!INSTANCE_MAP[className] || (INSTANCE_MAP[className]->flags ^ (INSTANCE_NOTCREATABLE | INSTANCE_SERVICE)) != 0) {
        return NoSuchService(className);
    }

    if (services.count(className) != 0)
        return std::make_optional(std::dynamic_pointer_cast<Service>(services[className]));
    return (std::optional<std::shared_ptr<Service>>)std::nullopt;
}

std::shared_ptr<DataModel> DataModel::CloneModel() {
    RefState<_RefStatePropertyCell> state = std::make_shared<__RefState<_RefStatePropertyCell>>();
    std::shared_ptr<DataModel> newModel = DataModel::New();

    // Copy properties
    for (std::string property : GetProperties()) {
        PropertyMeta meta = GetPropertyMeta(property).expect();
        
        if (meta.flags & (PROP_READONLY | PROP_NOSAVE)) continue;

        // Update InstanceRef properties using map above
        if (meta.type == &Data::InstanceRef::TYPE) {
            std::weak_ptr<Instance> refWeak = GetPropertyValue(property).expect().get<Data::InstanceRef>();
            if (refWeak.expired()) continue;

            auto ref = refWeak.lock();
            auto remappedRef = state->remappedInstances[ref]; // TODO: I think this is okay? Maybe?? Add null check?
            
            if (remappedRef) {
                // If the instance has already been remapped, set the new value
                newModel->SetPropertyValue(property, Data::InstanceRef(remappedRef)).expect();
            } else {
                // Otheriise, queue this property to be updated later, and keep its current value
                auto& refs = state->refsAwaitingRemap[ref];
                refs.push_back(std::make_pair(newModel, property));
                state->refsAwaitingRemap[ref] = refs;

                newModel->SetPropertyValue(property, Data::InstanceRef(ref)).expect();
            }
        } else {
            Data::Variant value = GetPropertyValue(property).expect();
            newModel->SetPropertyValue(property, value).expect();
        }
    }

    // Remap self
    state->remappedInstances[shared_from_this()] = newModel;

    // Remap queued properties
    for (std::pair<std::shared_ptr<Instance>, std::string> ref : state->refsAwaitingRemap[shared_from_this()]) {
        ref.first->SetPropertyValue(ref.second, Data::InstanceRef(newModel)).expect();
    }

    // Clone services
    for (std::shared_ptr<Instance> child : GetChildren()) {
        auto result = child->Clone(state);
        if (!result)
            continue;

        newModel->AddChild(result.value());

        // Special case: Ignore instances parented to DataModel which are not services
        if (child->GetClass()->flags & INSTANCE_SERVICE) {
            newModel->services[child->GetClass()->className] = std::dynamic_pointer_cast<Service>(result.value());            
        }
    }

    return newModel;
}