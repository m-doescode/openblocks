#include "datamodel.h"
#include "objects/base/service.h"
#include "objects/base/instance.h"
#include "objects/base/refstate.h"
#include "objects/base/service.h"
#include "objects/meta.h"
#include "objects/service/script/serverscriptservice.h"
#include "datatypes/variant.h"
#include "objects/service/workspace.h"
#include "objects/service/players.h"
#include "logger.h"
#include "panic.h"
#include <pugixml.hpp>
#include <cstdio>
#include <fstream>
#include <memory>
#include <optional>

DataModel::DataModel()
    : Instance(&TYPE) {
    this->name = "Place";
}

void DataModel::Init(bool runMode) {
    // Create default services
    GetService<Workspace>();
    GetService<Players>();
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

    for (std::shared_ptr<Instance> child : this->GetChildren()) {
        child->Serialize(root);
    }

    doc.save(outStream);
    currentFile = target;
    name = target;
    Logger::info("Place saved successfully");
}

std::shared_ptr<DataModel> DataModel::LoadFromFile(std::string path) {
    std::ifstream inStream(path);
    pugi::xml_document doc;
    doc.load(inStream);

    pugi::xml_node rootNode = doc.child("openblocks");
    std::shared_ptr<DataModel> newModel = std::make_shared<DataModel>();
    RefStateDeserialize state = std::make_shared<__RefStateDeserialize>();

    for (pugi::xml_node childNode : rootNode.children("Item")) {
        // Make sure the class hasn't already been deserialized
        std::string className = childNode.attribute("class").value();

        // TODO: Make this push its children into the first service, or however it is actually done in the thing
        // for parity
        if (newModel->services.count(className) != 0) {
            Logger::fatalErrorf("Service %s defined multiple times in file", className.c_str());
            continue;
        }

        auto result = Instance::Deserialize(childNode, state);
        if (result.isError()) {
            Logger::errorf("Failed to deserialize service: %s", result.errorMessage()->c_str());
            continue;
        }
        
        auto service = result.expect();
        newModel->AddChild(service);
        newModel->services[className] = std::dynamic_pointer_cast<Service>(service);
    }

    newModel->currentFile = path;
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
    services[className]->InitService();

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
    RefStateClone state = std::make_shared<__RefStateClone>();
    std::shared_ptr<DataModel> newModel = DataModel::New();

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
                newModel->SetProperty(property, InstanceRef(remappedRef)).expect();
            } else {
                // Otheriise, queue this property to be updated later, and keep its current value
                auto& refs = state->refsAwaitingRemap[ref];
                refs.push_back(std::make_pair(newModel, property));
                state->refsAwaitingRemap[ref] = refs;

                newModel->SetProperty(property, InstanceRef(ref)).expect();
            }
        } else {
            Variant value = GetProperty(property).expect();
            newModel->SetProperty(property, value).expect();
        }
    }

    // Remap self
    state->remappedInstances[shared_from_this()] = newModel;

    // Remap queued properties
    for (std::pair<std::shared_ptr<Instance>, std::string> ref : state->refsAwaitingRemap[shared_from_this()]) {
        ref.first->SetProperty(ref.second, InstanceRef(newModel)).expect();
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