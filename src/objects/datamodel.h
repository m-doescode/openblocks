#pragma once

#include "base.h"
#include <memory>

class Workspace;

class DataModel;
class Service;
extern std::map<std::string, std::function<std::shared_ptr<Service>(std::weak_ptr<DataModel>)>> SERVICE_CONSTRUCTORS;

// The root instance to all objects in the hierarchy
class DataModel : public Instance {
private:
    void DeserializeService(pugi::xml_node* node);
public:
    const static InstanceType TYPE;

    std::map<std::string, std::shared_ptr<Service>> services;

    std::optional<std::string> currentFile;

    DataModel();
    void Init();

    static inline std::shared_ptr<DataModel> New() { return std::make_shared<DataModel>(); };
    virtual const InstanceType* GetClass() override;

    template <typename T>
    std::shared_ptr<T> GetService(std::string name) {
        if (services.count(name) != 0)
            return services[name];
        // TODO: Validate name
        services[name] = SERVICE_CONSTRUCTORS[name](shared<DataModel>());
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