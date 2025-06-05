#pragma once

#include "base.h"
#include "error/data.h"
#include <memory>

class Instance;

class InstanceRef {
    std::weak_ptr<Instance> ref;
public:
    InstanceRef();
    InstanceRef(std::weak_ptr<Instance>);
    virtual ~InstanceRef();

    static const TypeDesc TYPE;

    operator std::weak_ptr<Instance>();

    virtual const std::string ToString() const;
    virtual void Serialize(pugi::xml_node node) const;
    virtual void PushLuaValue(lua_State*) const;
    static result<InstanceRef, DataParseError> Deserialize(pugi::xml_node node);
    static result<Variant, LuaCastError> FromLuaValue(lua_State*, int idx);
};