#include "datamodel.h"
#include "base/service.h"
#include "objects/base/instance.h"
#include "objects/base/service.h"
#include "workspace.h"
#include "logger.h"
#include "panic.h"
#include <cstdio>
#include <fstream>
#include <memory>
#include <functional>

// TODO: Move this to a different file
// ONLY add services here, all types are expected to inherit from Service, or errors WILL occur
std::map<std::string, std::function<std::shared_ptr<Service>(std::weak_ptr<DataModel>)>> SERVICE_CONSTRUCTORS = {
    { "Workspace", &Workspace::Create },
};

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

void DataModel::Init() {
    // Create the workspace if it doesn't exist
    if (this->services.count("Workspace") == 0)
        this->services["Workspace"] = std::make_shared<Workspace>(shared<DataModel>());
    
    // Init all services
    for (auto [_, service] : this->services) {
        service->InitService();
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
        child->Serialize(&root);
    }

    doc.save(outStream);
    currentFile = target;
    name = target;
    Logger::info("Place saved successfully");
}

void DataModel::DeserializeService(pugi::xml_node* node) {
    std::string className = node->attribute("class").value();
    if (SERVICE_CONSTRUCTORS.count(className) == 0) {
        Logger::fatalErrorf("Unknown service: '%s'", className.c_str());
        panic();
    }

    if (services.count(className) != 0) {
        Logger::fatalErrorf("Service %s defined multiple times in file", className.c_str());
        return;
    }

    // This will error if an abstract instance is used in the file. Oh well, not my prob rn.
    InstanceRef object = SERVICE_CONSTRUCTORS[className](shared<DataModel>());

    // Read properties
    pugi::xml_node propertiesNode = node->child("Properties");
    for (pugi::xml_node propertyNode : propertiesNode) {
        std::string propertyName = propertyNode.attribute("name").value();
        auto meta_ = object->GetPropertyMeta(propertyName);
        if (!meta_.has_value()) {
            Logger::fatalErrorf("Attempt to set unknown property '%s' of %s", propertyName.c_str(), object->GetClass()->className.c_str());
            continue;
        }
        Data::Variant value = Data::Variant::Deserialize(&propertyNode);
        object->SetPropertyValue(propertyName, value);
    }

    // Add children
    for (pugi::xml_node childNode : node->children("Item")) {
        InstanceRef child = Instance::Deserialize(&childNode);
        object->AddChild(child);
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
        newModel->DeserializeService(&childNode);
    }

    newModel->Init();

    return newModel;
}