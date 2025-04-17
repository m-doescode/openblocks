#pragma once

#include <iterator>
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <pugixml.hpp>

#include "error/instance.h"
#include "error/result.h"
#include "member.h"

class Instance;
typedef std::shared_ptr<Instance>(*InstanceConstructor)();

class DataModel;
class Workspace;

typedef int InstanceFlags;
// This instance should only be instantiated in special circumstances (i.e. by DataModel) and should be creatable directly via any API 
const InstanceFlags INSTANCE_NOTCREATABLE = (InstanceFlags)0x1;
// This instance is a service
const InstanceFlags INSTANCE_SERVICE = (InstanceFlags)0x2;

// Struct describing information about an instance
struct InstanceType {
    const InstanceType* super; // May be null
    std::string className;
    InstanceConstructor constructor;
    std::string explorerIcon = "";
    InstanceFlags flags;
};


class DescendantsIterator;

// Base class for all instances in the data model
                 // Note: enable_shared_from_this HAS to be public or else its field will not be populated
                 // Maybe this could be replaced with a friendship? But that seems unnecessary.
                 // https://stackoverflow.com/q/56415222/16255372
class Instance : public std::enable_shared_from_this<Instance> {
private:
    std::weak_ptr<Instance> parent;
    std::vector<std::shared_ptr<Instance>> children;

    std::optional<std::vector<std::string>> cachedMemberList;

    std::weak_ptr<DataModel> _dataModel;
    std::weak_ptr<Workspace> _workspace;

    bool ancestryContinuityCheck(std::optional<std::shared_ptr<Instance>> newParent);
    void updateAncestry(std::optional<std::shared_ptr<Instance>> child, std::optional<std::shared_ptr<Instance>> newParent);
protected:
    bool parentLocked = false;
    std::unique_ptr<MemberMap> memberMap;

    Instance(const InstanceType*);
    virtual ~Instance();

    virtual void OnParentUpdated(std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent);
    virtual void OnAncestryChanged(std::optional<std::shared_ptr<Instance>> child, std::optional<std::shared_ptr<Instance>> newParent);
    virtual void OnWorkspaceAdded(std::optional<std::shared_ptr<Workspace>> oldWorkspace, std::shared_ptr<Workspace> newWorkspace);
    virtual void OnWorkspaceRemoved(std::optional<std::shared_ptr<Workspace>> oldWorkspace);

    // The root data model this object is a descendant of
    std::optional<std::shared_ptr<DataModel>> dataModel();
    // The root workspace this object is a descendant of
    // NOTE: This value is not necessarily present if dataModel is present
    // Objects under services other than workspace will NOT have this field set
    std::optional<std::shared_ptr<Workspace>> workspace();

    template <typename T> inline std::shared_ptr<T> shared() { return std::dynamic_pointer_cast<T>(this->shared_from_this()); }
public:
    const static InstanceType TYPE;
    std::string name;

    // Instance is abstract, so it should not implement GetClass directly
    virtual const InstanceType* GetClass() = 0;
    bool SetParent(std::optional<std::shared_ptr<Instance>> newParent);
    std::optional<std::shared_ptr<Instance>> GetParent();
    bool IsParentLocked();
    inline const std::vector<std::shared_ptr<Instance>> GetChildren() { return children; }
    
    DescendantsIterator GetDescendantsStart();
    DescendantsIterator GetDescendantsEnd();
    // Utility functions
    inline void AddChild(std::shared_ptr<Instance> object) { object->SetParent(this->shared_from_this()); }

    // Properties
    result<Data::Variant, MemberNotFound> GetPropertyValue(std::string name);
    fallible<MemberNotFound, AssignToReadOnlyMember> SetPropertyValue(std::string name, Data::Variant value);
    result<PropertyMeta, MemberNotFound> GetPropertyMeta(std::string name);
    // Manually trigger the update of a property. Useful internally when setting properties directly
    void UpdateProperty(std::string name);
    // Returning a list of property names feels kinda janky. Is this really the way to go?
    std::vector<std::string> GetProperties();

    // Serialization
    void Serialize(pugi::xml_node parent);
    static result<std::shared_ptr<Instance>, NoSuchInstance> Deserialize(pugi::xml_node node);
};

typedef std::shared_ptr<Instance> InstanceRef;
typedef std::weak_ptr<Instance> InstanceRefWeak;

// https://gist.github.com/jeetsukumaran/307264
class DescendantsIterator {
public:
    typedef DescendantsIterator self_type;
    typedef std::shared_ptr<Instance> value_type;
    typedef std::shared_ptr<Instance>& reference;
    typedef std::shared_ptr<Instance> pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef int difference_type;

    DescendantsIterator(std::shared_ptr<Instance> current);
    inline self_type operator++() { self_type i = *this; ++*this; return i; }
    inline std::shared_ptr<Instance> operator*() { return current; }
    inline std::shared_ptr<Instance> operator->() { return current; }
    inline bool operator==(const self_type& rhs) { return current == rhs.current; }
    inline bool operator!=(const self_type& rhs) { return current != rhs.current; }

    self_type operator++(int _);
private:
    std::optional<std::shared_ptr<Instance>> root;
    std::shared_ptr<Instance> current;
    std::vector<int> siblingIndex;
};