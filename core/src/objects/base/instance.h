#pragma once

#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "objectmodel/type.h"
#include "datatypes/signal.h"
#include "error/instance.h"
#include "error/result.h"
#include "member.h"
#include "objects/base/refstate.h"
#include "utils.h"

// Used exclusively by Instance
#define INSTANCE_HEADER_APEX \
public: \
    static const InstanceType& Type(); \
private:

class Instance;

class DataModel;
class Workspace;

namespace pugi { class xml_node; };

class DescendantsIterator;
class JointInstance;

// Base class for all instances in the data model
                 // Note: enable_shared_from_this HAS to be public or else its field will not be populated
                 // Maybe this could be replaced with a friendship? But that seems unnecessary.
                 // https://stackoverflow.com/q/56415222/16255372
class Instance : public std::enable_shared_from_this<Instance> {
    INSTANCE_HEADER_APEX
private:
    std::weak_ptr<Instance> parent;
    std::vector<std::shared_ptr<Instance>> children;

    std::optional<std::vector<std::string>> cachedMemberList;

    std::weak_ptr<DataModel> _dataModel;
    std::weak_ptr<Workspace> _workspace;

    bool ancestryContinuityCheck(nullable std::shared_ptr<Instance> newParent);
    void updateAncestry(nullable std::shared_ptr<Instance> child, nullable std::shared_ptr<Instance> newParent);

    void ScriptRemove();

    friend JointInstance; // This isn't ideal, but oh well
protected:
    bool parentLocked = false;

    virtual ~Instance();

    virtual result<Variant, MemberNotFound> InternalGetPropertyValue(std::string name);
    virtual fallible<MemberNotFound, AssignToReadOnlyMember> InternalSetPropertyValue(std::string name, Variant value);
    virtual result<PropertyMeta, MemberNotFound> InternalGetPropertyMeta(std::string name);
    virtual void InternalUpdateProperty(std::string name);
    virtual std::vector<std::string> InternalGetProperties();

    virtual void OnParentUpdated(nullable std::shared_ptr<Instance> oldParent, nullable std::shared_ptr<Instance> newParent);
    virtual void OnAncestryChanged(nullable std::shared_ptr<Instance> child, nullable std::shared_ptr<Instance> newParent);
    virtual void OnWorkspaceAdded(nullable std::shared_ptr<Workspace> oldWorkspace, std::shared_ptr<Workspace> newWorkspace);
    virtual void OnWorkspaceRemoved(std::shared_ptr<Workspace> oldWorkspace);

    // The root data model this object is a descendant of
    nullable std::shared_ptr<DataModel> dataModel();
    // The root workspace this object is a descendant of
    // NOTE: This value is not necessarily present if dataModel is present
    // Objects under services other than workspace will NOT have this field set
    nullable std::shared_ptr<Workspace> workspace();
public:
    // Instance is abstract, so it should not implement GetType directly
    virtual const InstanceType& GetType() = 0;

    std::string name;

    // Signals
    SignalSource AncestryChanged;
    
    template <typename T> inline std::shared_ptr<T> shared() { return std::dynamic_pointer_cast<T>(this->shared_from_this()); }

    // Lua
    static void PushLuaLibrary(lua_State*); // Defined in lua/instancelib.cpp

    // Object hierarchy
    bool SetParent(nullable std::shared_ptr<Instance> newParent);
    inline void AddChild(std::shared_ptr<Instance> object) { object->SetParent(this->shared_from_this()); }
    nullable std::shared_ptr<Instance> GetParent();
    bool IsParentLocked();
    std::vector<std::shared_ptr<Instance>> GetChildren();
    std::vector<std::shared_ptr<Instance>> GetDescendants();
    void Destroy();
    template <typename T> bool IsA() { return IsA(T::Type().className); }
    [[deprecated]] DescendantsIterator GetDescendantsStart();
    [[deprecated]] DescendantsIterator GetDescendantsEnd();
    std::string GetFullName();
    
    nullable std::shared_ptr<Instance> FindFirstChild(std::string);
    nullable std::shared_ptr<Instance> FindFirstChildWhichIsA(std::string);
    template <typename T> nullable std::shared_ptr<T> FindFirstChildWhichIsA() { return std::dynamic_pointer_cast<T>(FindFirstChildWhichIsA(T::Type().className)); };
    nullable std::shared_ptr<Instance> FindFirstChildOfClass(std::string);
    
    nullable std::shared_ptr<Instance> FindFirstAncestor(std::string);
    nullable std::shared_ptr<Instance> FindFirstAncestorWhichIsA(std::string);
    template <typename T> nullable std::shared_ptr<T> FindFirstAncestorWhichIsA() { return std::dynamic_pointer_cast<T>(FindFirstAncestorWhichIsA(T::Type().className)); };
    nullable std::shared_ptr<Instance> FindFirstAncestorOfClass(std::string);

    // Script aliases
    inline nullable std::shared_ptr<Instance> ScriptFindFirstChildWhichIsA(std::string className) { return FindFirstChildWhichIsA(className); }
    inline nullable std::shared_ptr<Instance> ScriptFindFirstAncestorWhichIsA(std::string className) { return FindFirstAncestorWhichIsA(className); }

    // Determines whether this object is an instance of, or an instance of a subclass of the sepcified type's class name
    bool IsA(std::string className);
    bool IsAncestorOf(std::shared_ptr<Instance> descendant);
    bool IsDescendantOf(std::shared_ptr<Instance> ancestor);
    
    // Dynamically create an instance
    static result<std::shared_ptr<Instance>, NoSuchInstance, NotCreatableInstance> New(std::string className);

    // Properties
    result<Variant, MemberNotFound> GetProperty(std::string name);
    fallible<MemberNotFound, AssignToReadOnlyMember> SetProperty(std::string name, Variant value, bool sendUpdateEvent = true);
    result<PropertyMeta, MemberNotFound> GetPropertyMeta(std::string name);
    // Manually trigger the update of a property. Useful internally when setting properties directly
    void UpdateProperty(std::string name);
    // Returning a list of property names feels kinda janky. Is this really the way to go?
    std::vector<std::string> GetProperties();
    std::vector<std::pair<std::string, std::shared_ptr<Instance>>> GetReferenceProperties();

    template <typename T>
    result<std::shared_ptr<T>, InstanceCastError> CastTo() {
        // TODO: Too lazy to implement a manual check
        std::shared_ptr<T> result = std::dynamic_pointer_cast<T>(shared_from_this());
        if (result == nullptr)
            return InstanceCastError(GetType().className, T::Type().className);
        return result;
    }

    // Serialization
    void Serialize(pugi::xml_node parent, RefStateSerialize state = {});
    static result<std::shared_ptr<Instance>, NoSuchInstance> Deserialize(pugi::xml_node node, RefStateDeserialize state = {});
    nullable std::shared_ptr<Instance> Clone(RefStateClone state = {});
    inline nullable std::shared_ptr<Instance> ScriptClone() { return Clone(); };
};

// https://gist.github.com/jeetsukumaran/307264
class [[deprecated]] DescendantsIterator {
public:
    typedef DescendantsIterator self_type;
    typedef std::shared_ptr<Instance> value_type;
    typedef std::shared_ptr<Instance>& reference;
    typedef std::shared_ptr<Instance> pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef int difference_type;

    DescendantsIterator(std::shared_ptr<Instance> current);
    inline self_type operator++() { (*this)++; return (*this); }
    inline std::shared_ptr<Instance> operator*() { return current; }
    inline std::shared_ptr<Instance> operator->() { return current; }
    inline bool operator==(const self_type& rhs) { return current == rhs.current; }
    inline bool operator!=(const self_type& rhs) { return current != rhs.current; }

    self_type operator++(int _);
private:
    nullable std::shared_ptr<Instance> root;
    std::shared_ptr<Instance> current;
    std::vector<int> siblingIndex;
};

template <typename T, typename... Args>
std::enable_if_t<std::is_base_of_v<Instance, T>, std::shared_ptr<T>> new_instance(Args... args) {
    std::shared_ptr<T> obj = std::make_shared<T>(args...);
    if (obj->name == "") obj->name = T::Type().className;
    return obj;
}