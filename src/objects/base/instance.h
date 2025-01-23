#pragma once

#include "metadata.h"
#include <memory>
#include <optional>
#include <string>

// Struct describing information about an instance
struct InstanceType {
    InstanceType* super; // May be null
    std::string className;
    InstanceConstructor constructor;
    std::string explorerIcon = "";
};

// Base class for all instances in the data model
                 // Note: enable_shared_from_this HAS to be public or else its field will not be populated
                 // Maybe this could be replaced with a friendship? But that seems unnecessary.
                 // https://stackoverflow.com/q/56415222/16255372
class Instance : public std::enable_shared_from_this<Instance> {
private:
    std::optional<std::weak_ptr<Instance>> parent;
    std::vector<std::shared_ptr<Instance>> children;
protected:
    Instance(InstanceType*);
    virtual ~Instance();

    virtual void OnParentUpdated(std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent);
public:
    static InstanceType* TYPE;
    std::string name;

    // Instance is abstract, so it should not implement GetClass directly
    virtual InstanceType* GetClass() = 0;
    void SetParent(std::optional<std::shared_ptr<Instance>> newParent);
    std::optional<std::shared_ptr<Instance>> GetParent();
    inline const std::vector<std::shared_ptr<Instance>> GetChildren() { return children; }

    // Utility functions
    inline void AddChild(std::shared_ptr<Instance> object) { object->SetParent(this->shared_from_this()); }
};

typedef std::shared_ptr<Instance> InstanceRef;
typedef std::weak_ptr<Instance> InstanceRefWeak;