#pragma once

#include "base.h"
#include "error/result.h"
#include "logger.h"
#include "objects/base/instance.h"
#include "objects/meta.h"
#include "panic.h"
#include <memory>
#include <variant>

class Workspace;

class DataModel;
class Service;

class NoSuchService : public Error {
    public:
    inline NoSuchService(std::string className) : Error("Cannot insert service of unknown type " + className) {}
};

class ServiceAlreadyExists : public Error {
    public:
    inline ServiceAlreadyExists(std::string className) : Error("Service " + className + " is already inserted") {}
};

// The root instance to all objects in the hierarchy
class DataModel : public Instance {
private:
    void DeserializeService(pugi::xml_node node);
public:
    const static InstanceType TYPE;

    std::map<std::string, std::shared_ptr<Service>> services;

    std::optional<std::string> currentFile;

    DataModel();
    void Init();

    static inline std::shared_ptr<DataModel> New() { return std::make_shared<DataModel>(); };
    virtual const InstanceType* GetClass() override;

    // Inserts a service if it doesn't already exist
    fallible<ServiceAlreadyExists, NoSuchService> InsertService(std::string name) {
        if (services.count(name) != 0)
            return fallible<ServiceAlreadyExists, NoSuchService>(ServiceAlreadyExists(name));

        if (!INSTANCE_MAP[name] || (INSTANCE_MAP[name]->flags ^ (INSTANCE_NOTCREATABLE | INSTANCE_SERVICE)) != 0) {
            Logger::fatalErrorf("Attempt to create instance of unknown type %s", name);
            panic();
        }

        services[name] = std::dynamic_pointer_cast<Service>(INSTANCE_MAP[name]->constructor());
        AddChild(std::dynamic_pointer_cast<Instance>(services[name]));

        return {};
    }

    template <typename T>
    result<std::shared_ptr<T>, NoSuchService> GetService(std::string name) {
        if (services.count(name) != 0)
            return services[name];
        
        // TODO: Replace this with a result return type
        if (!INSTANCE_MAP[name] || (INSTANCE_MAP[name]->flags ^ (INSTANCE_NOTCREATABLE | INSTANCE_SERVICE)) != 0) {
            return NoSuchService(name);
        }

        services[name] = std::dynamic_pointer_cast<Service>(INSTANCE_MAP[name]->constructor());
        AddChild(std::dynamic_pointer_cast<Instance>(services[name]));

        return services[name];
    }

    template <typename T>
    std::optional<std::shared_ptr<T>> FindService() {
        if (services.count(name) != 0)
            return services[name];
        return std::nullopt;
    }

    // Saving/loading
    inline bool HasFile() { return this->currentFile.has_value(); }
    void SaveToFile(std::optional<std::string> path = std::nullopt);
    static std::shared_ptr<DataModel> LoadFromFile(std::string path);
};