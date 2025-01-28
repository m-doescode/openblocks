#pragma once

#include <map>
#include <memory>
#include <optional>
#include <variant>

class Instance;

struct PropertyMeta {
    void* backingField;
};

typedef std::variant<PropertyMeta> MemberMeta;

struct MemberMap {
    std::optional<std::unique_ptr<MemberMap>> super;
    std::map<std::string, PropertyMeta> members;
};

struct MemberNotFound {};