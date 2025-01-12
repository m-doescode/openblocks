#pragma once

#include "metadata.h"
#include <memory>
#include <string>

// Struct describing information about an instance
struct InstanceType {
    InstanceType* super; // May be null
    std::string className;
    InstanceConstructor constructor;
};

// Base class for all instances in the data model
class Instance : std::enable_shared_from_this<Instance> {
private:
    std::optional<std::weak_ptr<Instance>> parent;
    std::vector<std::shared_ptr<Instance>> children;
public:
    static InstanceType* TYPE;
    std::string name;

    InstanceType* GetClass();
    void SetParent(std::optional<std::shared_ptr<Instance>> newParent);
    std::optional<std::shared_ptr<Instance>> GetParent();
    inline const std::vector<std::shared_ptr<Instance>> GetChildren() { return children; }
};

typedef std::shared_ptr<Instance> InstanceRef;
typedef std::weak_ptr<Instance> InstanceRefWeak;