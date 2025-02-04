#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <map>
#include <vector>
#include <../include/expected.hpp>

#include "member.h"

class Instance;
typedef std::shared_ptr<Instance>(*InstanceConstructor)();

// Struct describing information about an instance
struct InstanceType {
    const InstanceType* super; // May be null
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

    std::optional<std::vector<std::string>> cachedMemberList;

    bool ancestryContinuityCheck(std::optional<std::shared_ptr<Instance>> newParent);
protected:
    bool parentLocked = false;
    std::unique_ptr<MemberMap> memberMap;

    Instance(const InstanceType*);
    virtual ~Instance();

    virtual void OnParentUpdated(std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent);

    template <typename T> inline std::shared_ptr<T> shared() { return std::dynamic_pointer_cast<T>(this->shared_from_this()); }
public:
    const static InstanceType TYPE;
    std::string name;

    // Instance is abstract, so it should not implement GetClass directly
    virtual const InstanceType* GetClass() = 0;
    bool SetParent(std::optional<std::shared_ptr<Instance>> newParent);
    std::optional<std::shared_ptr<Instance>> GetParent();
    inline const std::vector<std::shared_ptr<Instance>> GetChildren() { return children; }

    // Utility functions
    inline void AddChild(std::shared_ptr<Instance> object) { object->SetParent(this->shared_from_this()); }

    // Properties
    // Do I like using expected?
    tl::expected<Data::Variant, MemberNotFound> GetPropertyValue(std::string name);
    tl::expected<void, MemberNotFound> SetPropertyValue(std::string name, Data::Variant value);
    tl::expected<PropertyMeta, MemberNotFound> GetPropertyMeta(std::string name);
    // Returning a list of property names feels kinda janky. Is this really the way to go?
    std::vector<std::string> GetProperties();
};

typedef std::shared_ptr<Instance> InstanceRef;
typedef std::weak_ptr<Instance> InstanceRefWeak;