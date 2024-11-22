#pragma once

#include <optional>
#include <string>
#include <variant>
#include <map>
#include <vector>

#include "../../datatype.h"

class Instance;
typedef Instance(*InstanceConstructor)();

const uint INST_NOT_CREATABLE = 1;
// const uint INST_SINGLETON = 2;

typedef uint InstanceClassFlags;

struct InstanceClassDescriptor {
    std::string className;
    InstanceConstructor constructor;
    InstanceClassFlags flags;
};

//

const uint PROP_READONLY = 1;
const uint PROP_NOSCRIPT = 2; // Cannot be read or written to by unpriveleged scripts
const uint PROP_NOSAVE = 4; // Property should not be serialized by the engine
const uint PROP_CLIENTONLY = 8; // Only accessible by the client

typedef uint PropertyFlags;

typedef DataValue(*InstancePropertyGetter)();
typedef void(*InstancePropertySetter)(DataValue);

// Properties may either have a backing field directly accessed by Instance,
// or, for more granular control may define a getter (and setter if writable).
struct MemberPropertyDescriptor {
    DataType dataType;
    PropertyFlags flags;
    std::optional<DataValue*> backingField;
    std::optional<InstancePropertyGetter> getter;
    std::optional<InstancePropertySetter> setter;
};

//

typedef DataValue(*InstanceMethodHandler)(std::vector<DataValue>);

struct MemberMethodDescriptor {
    DataType returnType;
    // This may need to be converted into a vector in the future for overloaded methods
    std::vector<DataType> parameters;
    InstanceMethodHandler handler;
};

// TODO: Add MemberCallbackDescriptor
typedef std::map<std::string, std::variant<MemberPropertyDescriptor, MemberMethodDescriptor /*, MemberEventDescriptor */ /* , MemberCallbackDescriptor */>> InstanceMemberTable;