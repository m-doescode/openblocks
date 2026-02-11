#pragma once

#include "base.h"
#include "error/data.h"
#include "utils.h"
#include <memory>

class Instance;

class InstanceRef {
    nullable std::shared_ptr<Instance> ref;
public:
    InstanceRef();
    InstanceRef(std::weak_ptr<Instance>);
    virtual ~InstanceRef();

    static const TypeDesc TYPE;

    // TODO: Is this a good idea?
    template <typename T>
    operator std::shared_ptr<T>() { return std::dynamic_pointer_cast<T>(ref); }
    template <typename T>
    operator std::weak_ptr<T>() { return std::dynamic_pointer_cast<T>(ref); }

    virtual const std::string ToString() const;
    virtual void Serialize(pugi::xml_node node) const;
    virtual void PushLuaValue(lua_State*) const;
    static result<InstanceRef, DataParseError> Deserialize(pugi::xml_node node);
    static result<Variant, LuaCastError> FromLuaValue(lua_State*, int idx);

    bool operator ==(InstanceRef) const;
};