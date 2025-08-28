#pragma once

#include "error/instance.h"
#include "error/result.h"
#include "objects/annotation.h"
#include "objects/base/instance.h"
#include "objects/base/refstate.h"
#include <memory>

class Workspace;

class DataModel;
class Service;

// The root instance to all objects in the hierarchy
class DEF_INST_(abstract) DataModel : public Instance {
    AUTOGEN_PREAMBLE
private:
    // void DeserializeService(pugi::xml_node node, RefStateDeserialize);
    static void cloneService(std::shared_ptr<DataModel> target, std::shared_ptr<Service>, RefStateClone);
public:
    std::map<std::string, std::shared_ptr<Service>> services;

    std::optional<std::string> currentFile;

    DataModel();
    void Init(bool runMode = false);

    static inline std::shared_ptr<DataModel> New() { return std::make_shared<DataModel>(); };

    result<std::shared_ptr<Service>, NoSuchService> GetService(std::string className);
    result<nullable std::shared_ptr<Service>, NoSuchService> FindService(std::string className);

    template <typename T>
    std::shared_ptr<T> GetService() {
        auto result = GetService(T::TYPE.className);
        return std::dynamic_pointer_cast<T>(result.expect("GetService<T>() was called with a non-service instance type"));
    }

    template <typename T>
    nullable std::shared_ptr<T> FindService() {
        auto result = FindService(T::TYPE.className).expect("FindService<T>() was called with a non-service instance type");
        if (!result) return nullptr;
        return std::dynamic_pointer_cast<T>(result);
    }

    // Saving/loading
    inline bool HasFile() { return this->currentFile.has_value(); }
    void SaveToFile(std::optional<std::string> path = std::nullopt);
    static std::shared_ptr<DataModel> LoadFromFile(std::string path);
    std::shared_ptr<DataModel> CloneModel();
};